/*
 * AstroDefender - Core Game Logic
 * game.c
 *
 * Handles state machine, update loop, collision detection,
 * progression, persistence, lightweight audio feedback, and enemy AI.
 */

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "game.h"
#include "config.h"
#include "renderer.h"

#define ARRAY_LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))

typedef struct {
    const char* name;
    float player_speed;
    float enemy_speed;
    float bullet_speed;
    int shoot_chance_divisor;
    int reward_percent;
    int lives;
    int event_frequency_percent;
} DifficultyConfig;

typedef struct {
    unsigned int magic;
    unsigned int version;
    ScoreTable scores;
    PlayerStats stats;
    AchievementState achievements;
    Settings settings;
} SaveData;

static SDL_AudioDeviceID g_audio_device = 0;
static int g_audio_failed = 0;

static const DifficultyConfig DIFFICULTY_CONFIGS[DIFFICULTY_COUNT] = {
    { "EASY",      1.12f, 0.78f, 0.80f, 155,  80, 4,  80 },
    { "NORMAL",    1.00f, 1.00f, 1.00f, 110, 100, 3, 100 },
    { "HARD",      0.96f, 1.26f, 1.22f,  78, 135, 3, 125 },
    { "NIGHTMARE", 0.92f, 1.55f, 1.48f,  52, 180, 2, 150 }
};

static const char* ACH_NAMES[ACH_COUNT] = {
    "FIRST BLOOD", "COMBO MASTER", "WAVE RIDER", "BONUS HUNTER", "NIGHTMARE ACE"
};

static const char* ACH_DESC[ACH_COUNT] = {
    "Destroy your first invader", "Reach a 10x combo", "Clear 3 waves total",
    "Destroy 5 bonus ships", "Clear a wave on Nightmare"
};

static int load_save_data(SaveData* data)
{
    FILE* f = fopen(SAVE_FILE_NAME, "rb");
    if (!f) return 0;
    memset(data, 0, sizeof(*data));
    size_t got = fread(data, 1, sizeof(*data), f);
    fclose(f);
    return got == sizeof(*data) && data->magic == SAVE_MAGIC && data->version == SAVE_VERSION;
}

static void save_context(const GameContext* ctx)
{
    SaveData data;
    memset(&data, 0, sizeof(data));
    data.magic = SAVE_MAGIC;
    data.version = SAVE_VERSION;
    data.scores = ctx->scores;
    data.stats = ctx->stats;
    data.achievements = ctx->achievements;
    data.settings = ctx->settings;
    FILE* f = fopen(SAVE_FILE_NAME, "wb");
    if (!f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to save %s", SAVE_FILE_NAME);
        return;
    }
    if (fwrite(&data, 1, sizeof(data), f) != sizeof(data)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to write complete save file");
    }
    fclose(f);
}

/* -------------------------------------------------------
 * Utility helpers
 * ------------------------------------------------------- */

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static const DifficultyConfig* difficulty_config(const GameContext* ctx)
{
    return &DIFFICULTY_CONFIGS[clamp_int((int)ctx->difficulty, 0, DIFFICULTY_COUNT - 1)];
}

static void set_status(GameContext* ctx, const char* text)
{
    if (!text) return;
    snprintf(ctx->status_message, sizeof(ctx->status_message), "%s", text);
    ctx->status_timer = 160;
}

static void add_screen_shake(GameContext* ctx, int strength, int frames)
{
    if (!ctx->settings.screen_shake) return;
    ctx->screen_shake_strength = clamp_int(strength, 1, 12);
    if (ctx->screen_shake_timer < frames) ctx->screen_shake_timer = frames;
}

static void spawn_floating_text(GameContext* ctx, float x, float y, const char* text,
                                Uint8 r, Uint8 g, Uint8 b)
{
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        FloatingText* ft = &ctx->floating_texts[i];
        if (!ft->active) {
            ft->pos.x = x;
            ft->pos.y = y;
            ft->active = 1;
            ft->life = 70;
            snprintf(ft->text, sizeof(ft->text), "%s", text);
            ft->r = r; ft->g = g; ft->b = b;
            return;
        }
    }
}

static void unlock_achievement(GameContext* ctx, AchievementId id)
{
    if (id < 0 || id >= ACH_COUNT || ctx->achievements.unlocked[id]) return;
    ctx->achievements.unlocked[id] = 1;
    ctx->last_reward_points += 250;
    set_status(ctx, ACH_NAMES[id]);
    spawn_floating_text(ctx, SCREEN_WIDTH / 2.0f - 80.0f, 96.0f, "ACHIEVEMENT +250", 255, 220, 60);
    Game_PlayTone(880, 120, ctx->settings.sfx_volume);
}


static int current_daily_seed(void)
{
    return (int)(time(NULL) / 86400);
}

static void refresh_daily_challenge(GameContext* ctx)
{
    int seed = current_daily_seed();
    if (ctx->stats.daily_seed != seed) {
        ctx->stats.daily_seed = seed;
        ctx->stats.daily_best = 0;
    }
}

static void update_achievements(GameContext* ctx)
{
    if (ctx->stats.enemies_destroyed >= 1) unlock_achievement(ctx, ACH_FIRST_BLOOD);
    if (ctx->best_combo_this_run >= 10 || ctx->stats.best_combo >= 10) unlock_achievement(ctx, ACH_COMBO_MASTER);
    if (ctx->stats.waves_cleared >= 3) unlock_achievement(ctx, ACH_WAVE_RIDER);
    if (ctx->stats.bonus_destroyed >= 5) unlock_achievement(ctx, ACH_BONUS_HUNTER);
    if (ctx->difficulty == DIFFICULTY_NIGHTMARE && ctx->stats.waves_cleared >= 1) unlock_achievement(ctx, ACH_NIGHTMARE);
}

static int apply_reward(GameContext* ctx, int base)
{
    int reward = base * difficulty_config(ctx)->reward_percent / 100;
    if (ctx->active_event == EVENT_SCORE_SURGE) reward *= 2;
    if (ctx->combo > 1) reward += (ctx->combo - 1) * SCORE_COMBO_STEP;
    return reward;
}

/* -------------------------------------------------------
 * Lightweight procedural audio
 * ------------------------------------------------------- */

void Game_PlayTone(int frequency, int duration_ms, int volume)
{
    if (g_audio_failed || volume <= 0 || frequency <= 0 || duration_ms <= 0) return;

    if (!g_audio_device) {
        SDL_AudioSpec want;
        SDL_zero(want);
        want.freq = 44100;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 1024;
        g_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        if (!g_audio_device) {
            SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "SDL_OpenAudioDevice failed: %s", SDL_GetError());
            g_audio_failed = 1;
            return;
        }
        SDL_PauseAudioDevice(g_audio_device, 0);
    }

    int samples = 44100 * duration_ms / 1000;
    Sint16* buffer = (Sint16*)SDL_malloc((size_t)samples * sizeof(Sint16));
    if (!buffer) return;
    float amplitude = (float)clamp_int(volume, 0, 100) / 100.0f * 6000.0f;
    for (int i = 0; i < samples; i++) {
        float t = (float)i / 44100.0f;
        float envelope = 1.0f - (float)i / (float)samples;
        buffer[i] = (Sint16)(sinf(2.0f * 3.14159265f * (float)frequency * t) * amplitude * envelope);
    }
    SDL_QueueAudio(g_audio_device, buffer, (Uint32)((size_t)samples * sizeof(Sint16)));
    SDL_free(buffer);
}

/* -------------------------------------------------------
 * Background and initialization
 * ------------------------------------------------------- */

static void init_stars(GameContext* ctx)
{
    for (int layer = 0; layer < STAR_LAYERS; layer++) {
        for (int i = 0; i < STARS_PER_LAYER; i++) {
            Star* s = &ctx->stars[layer][i];
            s->x = (float)(rand() % SCREEN_WIDTH);
            s->y = (float)(rand() % SCREEN_HEIGHT);
            s->speed = 0.25f + layer * 0.35f;
            s->brightness = (Uint8)(80 + layer * 55 + rand() % 60);
            s->size = layer + 1;
        }
    }
}

static void update_stars(GameContext* ctx)
{
    float event_boost = (ctx->active_event == EVENT_METEOR_STORM) ? 1.85f : 1.0f;
    for (int layer = 0; layer < STAR_LAYERS; layer++) {
        for (int i = 0; i < STARS_PER_LAYER; i++) {
            Star* s = &ctx->stars[layer][i];
            s->y += s->speed * event_boost;
            if (s->y > SCREEN_HEIGHT) {
                s->y = 0.0f;
                s->x = (float)(rand() % SCREEN_WIDTH);
            }
        }
    }
}

void Game_InitContext(GameContext* ctx)
{
    memset(ctx, 0, sizeof(GameContext));
    srand((unsigned int)time(NULL));

    ctx->state = STATE_MENU;
    ctx->running = 1;
    ctx->level = 1;
    ctx->enemy_direction = 1;
    ctx->difficulty = DIFFICULTY_NORMAL;
    ctx->milestone_next_score = SCORE_MILESTONE;
    ctx->settings.music_volume = 45;
    ctx->settings.sfx_volume = 70;
    ctx->settings.screen_shake = 1;
    ctx->settings.show_tutorial = 1;

    SaveData saved;
    if (load_save_data(&saved)) {
        ctx->scores = saved.scores;
        ctx->stats = saved.stats;
        ctx->achievements = saved.achievements;
        ctx->settings = saved.settings;
        ctx->settings.music_volume = clamp_int(ctx->settings.music_volume, 0, 100);
        ctx->settings.sfx_volume = clamp_int(ctx->settings.sfx_volume, 0, 100);
    } else {
        Scores_Load(&ctx->scores);
    }
    refresh_daily_challenge(ctx);
    if (ctx->scores.count > 0) ctx->hi_score = ctx->scores.entries[0].score;

    init_stars(ctx);
    Game_ResetPlayer(ctx);
}

void Game_ResetPlayer(GameContext* ctx)
{
    ctx->player.pos.x = (float)(SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2);
    ctx->player.pos.y = (float)(SCREEN_HEIGHT - PLAYER_HEIGHT - 50);
    ctx->player.active = 1;
    ctx->player.invincible = 0;
    ctx->player.shield_active = 0;
    ctx->player.last_shot_time = 0;
    ctx->player.rapid_until = 0;
    ctx->player.double_shot_until = 0;
}

static void reset_run(GameContext* ctx)
{
    ctx->score = 0;
    ctx->level = 1;
    ctx->combo = 0;
    ctx->best_combo_this_run = 0;
    ctx->milestone_next_score = SCORE_MILESTONE;
    ctx->last_reward_points = 0;
    ctx->player.lives = difficulty_config(ctx)->lives;
    refresh_daily_challenge(ctx);
    ctx->stats.games_played++;
    Game_ResetPlayer(ctx);
    Game_StartLevel(ctx);
    ctx->state = ctx->settings.show_tutorial ? STATE_TUTORIAL : STATE_PLAYING;
    set_status(ctx, "DEFEND EARTH");
}

void Game_StartLevel(GameContext* ctx)
{
    memset(ctx->player_bullets, 0, sizeof(ctx->player_bullets));
    memset(ctx->enemy_bullets, 0, sizeof(ctx->enemy_bullets));
    memset(ctx->powerups, 0, sizeof(ctx->powerups));
    memset(ctx->particles, 0, sizeof(ctx->particles));
    memset(ctx->floating_texts, 0, sizeof(ctx->floating_texts));

    ctx->enemies_alive = 0;
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            Enemy* e = &ctx->enemies[row][col];
            e->pos.x = (float)(ENEMY_START_X + col * ENEMY_PADDING_X);
            e->pos.y = (float)(ENEMY_START_Y + row * ENEMY_PADDING_Y);
            e->alive = 1;
            e->anim_frame = 0;
            e->anim_timer = 0;
            if (row == 0) e->type = ENEMY_TYPE_BOSS;
            else if (row <= 1) e->type = ENEMY_TYPE_C;
            else if (row <= 2) e->type = ENEMY_TYPE_B;
            else e->type = ENEMY_TYPE_A;
            ctx->enemies_alive++;
        }
    }

    float speed = (ENEMY_SPEED_INIT + (ctx->level - 1) * ENEMY_SPEED_STEP) * difficulty_config(ctx)->enemy_speed;
    ctx->enemy_dx = speed;
    ctx->enemy_direction = 1;
    ctx->enemy_move_interval = clamp_int(30 - (ctx->level - 1), 12, 30);
    ctx->enemy_move_timer = 0;

    ctx->bonus_ship.active = 0;
    ctx->active_event = EVENT_NONE;
    ctx->event_until = 0;
    ctx->next_bonus_time = SDL_GetTicks() + BONUS_SPAWN_INTERVAL;
    ctx->next_event_time = SDL_GetTicks() + RANDOM_EVENT_MIN_MS + (rand() % (RANDOM_EVENT_MAX_MS - RANDOM_EVENT_MIN_MS));
}

/* -------------------------------------------------------
 * Collision and effects
 * ------------------------------------------------------- */

int Game_RectOverlap(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh)
{
    return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
}

void Game_SpawnExplosion(GameContext* ctx, float x, float y, Uint8 r, Uint8 g, Uint8 b, int count)
{
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (!ctx->particles[i].life) {
            Particle* p = &ctx->particles[i];
            float angle = (float)(rand() % 628) / 100.0f;
            float speed = 0.5f + (float)(rand() % 30) / 10.0f;
            p->pos.x = x; p->pos.y = y;
            p->vel.x = cosf(angle) * speed;
            p->vel.y = sinf(angle) * speed;
            p->life = PARTICLE_LIFETIME - rand() % 15;
            p->max_life = p->life;
            p->r = r; p->g = g; p->b = b;
            p->size = 1.5f + (float)(rand() % 3);
            count--;
        }
    }
}

static void spawn_powerup(GameContext* ctx, float x, float y, int force)
{
    int drop_chance = (ctx->active_event == EVENT_SHIELD_DRIFT) ? 24 : 10;
    if (!force && (rand() % 100) >= drop_chance) return;
    for (int i = 0; i < MAX_POWERUPS; i++) {
        PowerUp* p = &ctx->powerups[i];
        if (!p->active) {
            p->active = 1;
            p->pos.x = x;
            p->pos.y = y;
            p->dy = POWERUP_FALL_SPEED;
            p->ttl = 600;
            p->type = (PowerUpType)(1 + rand() % (POWERUP_COUNT - 1));
            return;
        }
    }
}

static int fire_player_bullet(GameContext* ctx, float x)
{
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!ctx->player_bullets[i].active) {
            Bullet* b = &ctx->player_bullets[i];
            b->pos.x = x;
            b->pos.y = ctx->player.pos.y - BULLET_HEIGHT;
            b->dy = -PLAYER_BULLET_SPEED;
            b->active = 1;
            return 1;
        }
    }
    return 0;
}

static void player_shoot(GameContext* ctx)
{
    Uint32 now = SDL_GetTicks();
    Uint32 delay = (now < ctx->player.rapid_until) ? PLAYER_SHOOT_DELAY / 2 : PLAYER_SHOOT_DELAY;
    if (now - ctx->player.last_shot_time < delay) return;

    float center = ctx->player.pos.x + PLAYER_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
    int fired = 0;
    if (now < ctx->player.double_shot_until) {
        fired += fire_player_bullet(ctx, center - 10.0f);
        fired += fire_player_bullet(ctx, center + 10.0f);
    } else {
        fired += fire_player_bullet(ctx, center);
    }
    if (fired > 0) {
        ctx->player.last_shot_time = now;
        Game_PlayTone(520, 45, ctx->settings.sfx_volume / 2);
    }
}

static void enemy_try_shoot(GameContext* ctx, int row, int col)
{
    for (int r = ENEMY_ROWS - 1; r > row; r--) if (ctx->enemies[r][col].alive) return;

    int divisor = difficulty_config(ctx)->shoot_chance_divisor - (ctx->level * 4);
    if (ctx->active_event == EVENT_METEOR_STORM) divisor = divisor * 80 / 100;
    divisor = clamp_int(divisor, 28, 240);
    if (rand() % divisor != 0) return;

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!ctx->enemy_bullets[i].active) {
            Enemy* e = &ctx->enemies[row][col];
            Bullet* b = &ctx->enemy_bullets[i];
            b->pos.x = e->pos.x + ENEMY_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
            b->pos.y = e->pos.y + ENEMY_HEIGHT;
            b->dy = (ENEMY_BULLET_SPEED + (ctx->level - 1) * 0.3f) * difficulty_config(ctx)->bullet_speed;
            b->active = 1;
            break;
        }
    }
}

static void enter_game_over(GameContext* ctx)
{
    ctx->state = STATE_GAMEOVER;
    if (ctx->score > ctx->stats.daily_best) ctx->stats.daily_best = ctx->score;
    ctx->stats.total_score += ctx->score;
    if (ctx->best_combo_this_run > ctx->stats.best_combo) ctx->stats.best_combo = ctx->best_combo_this_run;
    update_achievements(ctx);
    if (Scores_IsHighScore(&ctx->scores, ctx->score)) {
        ctx->name_entry_active = 1;
        ctx->input_len = 0;
        memset(ctx->input_name, 0, sizeof(ctx->input_name));
    }
    save_context(ctx);
}

static void handle_enemy_destroyed(GameContext* ctx, Enemy* e)
{
    Uint32 now = SDL_GetTicks();
    e->alive = 0;
    ctx->enemies_alive--;
    ctx->stats.enemies_destroyed++;

    if (now - ctx->last_kill_time <= COMBO_WINDOW_MS) ctx->combo++;
    else ctx->combo = 1;
    ctx->last_kill_time = now;
    if (ctx->combo > ctx->best_combo_this_run) ctx->best_combo_this_run = ctx->combo;

    int base = SCORE_ENEMY_TYPE0;
    if (e->type == ENEMY_TYPE_B) base = SCORE_ENEMY_TYPE1;
    else if (e->type == ENEMY_TYPE_C) base = SCORE_ENEMY_TYPE2;
    else if (e->type == ENEMY_TYPE_BOSS) base = SCORE_ENEMY_TYPE3;
    int pts = apply_reward(ctx, base);
    ctx->score += pts;
    if (ctx->score > ctx->hi_score) ctx->hi_score = ctx->score;
    ctx->score_flash_timer = 20;

    char buf[32];
    snprintf(buf, sizeof(buf), "+%d x%d", pts, ctx->combo);
    spawn_floating_text(ctx, e->pos.x, e->pos.y, buf, 255, 240, 90);

    Uint8 er = 255, eg = 100, eb = 40;
    if (e->type == ENEMY_TYPE_BOSS) { er = 180; eg = 80; eb = 255; }
    else if (e->type == ENEMY_TYPE_C) { er = 80; eg = 255; eb = 120; }
    else if (e->type == ENEMY_TYPE_B) { er = 0; eg = 220; eb = 255; }
    Game_SpawnExplosion(ctx, e->pos.x + ENEMY_WIDTH / 2.0f, e->pos.y + ENEMY_HEIGHT / 2.0f, er, eg, eb, 16);
    add_screen_shake(ctx, 2, 8);
    spawn_powerup(ctx, e->pos.x + ENEMY_WIDTH / 2.0f, e->pos.y, 0);
    Game_PlayTone(660 + ctx->combo * 12, 55, ctx->settings.sfx_volume);

    ctx->enemy_move_interval = 5 + (ctx->enemies_alive * 25) / (ENEMY_ROWS * ENEMY_COLS);
    if (ctx->enemy_move_interval < 5) ctx->enemy_move_interval = 5;

    if (ctx->score >= ctx->milestone_next_score) {
        ctx->last_reward_points += 500;
        ctx->score += 500;
        ctx->milestone_next_score += SCORE_MILESTONE;
        set_status(ctx, "MILESTONE REWARD +500");
        spawn_powerup(ctx, ctx->player.pos.x + PLAYER_WIDTH / 2.0f, 110.0f, 1);
    }
    update_achievements(ctx);
}

static void collect_powerup(GameContext* ctx, PowerUp* p)
{
    Uint32 now = SDL_GetTicks();
    const char* label = "POWER UP";
    switch (p->type) {
        case POWERUP_SHIELD:
            ctx->player.shield_active = 1;
            ctx->player.shield_until = now + POWERUP_DURATION_MS;
            label = "SHIELD";
            break;
        case POWERUP_RAPID_FIRE:
            ctx->player.rapid_until = now + POWERUP_DURATION_MS;
            label = "RAPID FIRE";
            break;
        case POWERUP_DOUBLE_SHOT:
            ctx->player.double_shot_until = now + POWERUP_DURATION_MS;
            label = "DOUBLE SHOT";
            break;
        case POWERUP_REPAIR:
            if (ctx->player.lives < PLAYER_MAX_LIVES + 1) ctx->player.lives++;
            label = "REPAIR +1";
            break;
        default:
            break;
    }
    p->active = 0;
    set_status(ctx, label);
    spawn_floating_text(ctx, ctx->player.pos.x, ctx->player.pos.y - 28.0f, label, 80, 255, 220);
    Game_PlayTone(760, 90, ctx->settings.sfx_volume);
}

static void update_random_event(GameContext* ctx, Uint32 now)
{
    if (ctx->active_event != EVENT_NONE && now >= ctx->event_until) {
        ctx->active_event = EVENT_NONE;
        set_status(ctx, "SECTOR STABILIZED");
        ctx->next_event_time = now + RANDOM_EVENT_MIN_MS + (rand() % (RANDOM_EVENT_MAX_MS - RANDOM_EVENT_MIN_MS));
    }
    if (ctx->active_event == EVENT_NONE && now >= ctx->next_event_time) {
        int chance = difficulty_config(ctx)->event_frequency_percent;
        if ((rand() % 100) < chance) {
            ctx->active_event = (RandomEventType)(1 + rand() % (EVENT_COUNT - 1));
            ctx->event_until = now + RANDOM_EVENT_DURATION_MS;
            if (ctx->active_event == EVENT_METEOR_STORM) set_status(ctx, "RANDOM EVENT: METEOR STORM");
            else if (ctx->active_event == EVENT_SCORE_SURGE) set_status(ctx, "RANDOM EVENT: SCORE SURGE");
            else if (ctx->active_event == EVENT_SHIELD_DRIFT) set_status(ctx, "RANDOM EVENT: POWER DRIFT");
            Game_PlayTone(320, 160, ctx->settings.sfx_volume);
        }
        ctx->next_event_time = now + RANDOM_EVENT_MIN_MS;
    }
}

static void update_feedback(GameContext* ctx)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &ctx->particles[i];
        if (!p->life) continue;
        p->pos.x += p->vel.x;
        p->pos.y += p->vel.y;
        p->vel.y += 0.05f;
        p->life--;
    }
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        FloatingText* ft = &ctx->floating_texts[i];
        if (!ft->active) continue;
        ft->pos.y -= 0.45f;
        if (--ft->life <= 0) ft->active = 0;
    }
    if (ctx->score_flash_timer > 0) ctx->score_flash_timer--;
    if (ctx->life_lost_flash > 0) ctx->life_lost_flash--;
    if (ctx->screen_shake_timer > 0) ctx->screen_shake_timer--;
    if (ctx->status_timer > 0) ctx->status_timer--;
}

/* -------------------------------------------------------
 * Update: Playing State
 * ------------------------------------------------------- */

static void update_playing(GameContext* ctx)
{
    Uint32 now = SDL_GetTicks();
    update_random_event(ctx, now);

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float player_speed = PLAYER_SPEED * difficulty_config(ctx)->player_speed;
    if (ctx->player.active) {
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) ctx->player.pos.x -= player_speed;
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) ctx->player.pos.x += player_speed;
        ctx->player.pos.x = (float)clamp_int((int)ctx->player.pos.x, 0, SCREEN_WIDTH - PLAYER_WIDTH);
        if (keys[SDL_SCANCODE_SPACE]) player_shoot(ctx);

        if (ctx->player.invincible && now - ctx->player.hit_time > PLAYER_INVINCIBLE_MS) ctx->player.invincible = 0;
        if (ctx->player.shield_active && now >= ctx->player.shield_until) ctx->player.shield_active = 0;
    }

    if (ctx->combo > 0 && now - ctx->last_kill_time > COMBO_WINDOW_MS) ctx->combo = 0;

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        Bullet* b = &ctx->player_bullets[i];
        if (!b->active) continue;
        b->pos.y += b->dy;
        if (b->pos.y + BULLET_HEIGHT < 0) { b->active = 0; continue; }

        if (ctx->bonus_ship.active &&
            Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                             ctx->bonus_ship.pos.x, ctx->bonus_ship.pos.y, BONUS_SHIP_WIDTH, BONUS_SHIP_HEIGHT)) {
            b->active = 0;
            ctx->bonus_ship.active = 0;
            int pts = apply_reward(ctx, SCORE_BONUS_SHIP);
            ctx->score += pts;
            ctx->stats.bonus_destroyed++;
            if (ctx->score > ctx->hi_score) ctx->hi_score = ctx->score;
            ctx->score_flash_timer = 45;
            spawn_floating_text(ctx, ctx->bonus_ship.pos.x, ctx->bonus_ship.pos.y, "+BONUS", 255, 180, 40);
            Game_SpawnExplosion(ctx, ctx->bonus_ship.pos.x + BONUS_SHIP_WIDTH / 2.0f,
                                ctx->bonus_ship.pos.y + BONUS_SHIP_HEIGHT / 2.0f, 255, 160, 40, 28);
            add_screen_shake(ctx, 5, 16);
            spawn_powerup(ctx, ctx->bonus_ship.pos.x + BONUS_SHIP_WIDTH / 2.0f, ctx->bonus_ship.pos.y, 1);
            update_achievements(ctx);
            continue;
        }

        int bullet_done = 0;
        for (int row = 0; row < ENEMY_ROWS && !bullet_done; row++) {
            for (int col = 0; col < ENEMY_COLS; col++) {
                Enemy* e = &ctx->enemies[row][col];
                if (!e->alive) continue;
                if (Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                                     e->pos.x, e->pos.y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
                    b->active = 0;
                    handle_enemy_destroyed(ctx, e);
                    bullet_done = 1;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        Bullet* b = &ctx->enemy_bullets[i];
        if (!b->active) continue;
        b->pos.y += b->dy;
        if (b->pos.y > SCREEN_HEIGHT) { b->active = 0; continue; }
        if (ctx->player.active && !ctx->player.invincible &&
            Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                             ctx->player.pos.x + 4, ctx->player.pos.y + 4, PLAYER_WIDTH - 8, PLAYER_HEIGHT - 8)) {
            b->active = 0;
            if (ctx->player.shield_active) {
                ctx->player.shield_active = 0;
                Game_SpawnExplosion(ctx, ctx->player.pos.x + PLAYER_WIDTH / 2.0f, ctx->player.pos.y + PLAYER_HEIGHT / 2.0f,
                                    80, 220, 255, 18);
                set_status(ctx, "SHIELD ABSORBED HIT");
            } else {
                ctx->player.lives--;
                ctx->player.invincible = 1;
                ctx->player.hit_time = now;
                ctx->life_lost_flash = 45;
                ctx->combo = 0;
                Game_SpawnExplosion(ctx, ctx->player.pos.x + PLAYER_WIDTH / 2.0f, ctx->player.pos.y + PLAYER_HEIGHT / 2.0f,
                                    255, 80, 80, 24);
                add_screen_shake(ctx, 7, 22);
                Game_PlayTone(180, 180, ctx->settings.sfx_volume);
                if (ctx->player.lives <= 0) {
                    ctx->player.active = 0;
                    enter_game_over(ctx);
                    return;
                }
            }
        }
    }

    ctx->enemy_move_timer++;
    if (ctx->enemy_move_timer >= ctx->enemy_move_interval) {
        ctx->enemy_move_timer = 0;
        int hit_wall = 0;
        for (int row = 0; row < ENEMY_ROWS; row++) {
            for (int col = 0; col < ENEMY_COLS; col++) {
                Enemy* e = &ctx->enemies[row][col];
                if (!e->alive) continue;
                float nx = e->pos.x + ctx->enemy_dx * ctx->enemy_direction;
                if (nx < 8 || nx + ENEMY_WIDTH > SCREEN_WIDTH - 8) hit_wall = 1;
            }
        }
        if (hit_wall) {
            ctx->enemy_direction = -ctx->enemy_direction;
            for (int row = 0; row < ENEMY_ROWS; row++)
                for (int col = 0; col < ENEMY_COLS; col++)
                    if (ctx->enemies[row][col].alive) ctx->enemies[row][col].pos.y += ENEMY_DROP_DISTANCE;
        } else {
            for (int row = 0; row < ENEMY_ROWS; row++)
                for (int col = 0; col < ENEMY_COLS; col++)
                    if (ctx->enemies[row][col].alive) ctx->enemies[row][col].pos.x += ctx->enemy_dx * ctx->enemy_direction;
        }
        for (int row = 0; row < ENEMY_ROWS; row++)
            for (int col = 0; col < ENEMY_COLS; col++)
                if (ctx->enemies[row][col].alive) ctx->enemies[row][col].anim_frame ^= 1;
    }

    for (int row = 0; row < ENEMY_ROWS; row++)
        for (int col = 0; col < ENEMY_COLS; col++)
            if (ctx->enemies[row][col].alive) enemy_try_shoot(ctx, row, col);

    for (int i = 0; i < MAX_POWERUPS; i++) {
        PowerUp* p = &ctx->powerups[i];
        if (!p->active) continue;
        p->pos.y += p->dy;
        if (--p->ttl <= 0 || p->pos.y > SCREEN_HEIGHT) { p->active = 0; continue; }
        if (Game_RectOverlap(p->pos.x - 8, p->pos.y - 8, 16, 16,
                             ctx->player.pos.x, ctx->player.pos.y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            collect_powerup(ctx, p);
        }
    }

    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            if (ctx->enemies[row][col].alive && ctx->enemies[row][col].pos.y + ENEMY_HEIGHT >= ctx->player.pos.y) {
                enter_game_over(ctx);
                return;
            }
        }
    }

    if (ctx->enemies_alive == 0) {
        if (ctx->score > ctx->stats.daily_best) ctx->stats.daily_best = ctx->score;
        ctx->level++;
        ctx->stats.waves_cleared++;
        update_achievements(ctx);
        save_context(ctx);
        ctx->state = STATE_VICTORY;
        Game_PlayTone(980, 140, ctx->settings.sfx_volume);
        return;
    }

    if (!ctx->bonus_ship.active && now >= ctx->next_bonus_time) {
        ctx->bonus_ship.active = 1;
        ctx->bonus_ship.pos.y = 58.0f;
        ctx->bonus_ship.dx = BONUS_SHIP_SPEED * difficulty_config(ctx)->enemy_speed;
        ctx->bonus_ship.pos.x = -BONUS_SHIP_WIDTH;
        ctx->next_bonus_time = now + BONUS_SPAWN_INTERVAL + (rand() % 10000);
    }
    if (ctx->bonus_ship.active) {
        ctx->bonus_ship.pos.x += ctx->bonus_ship.dx;
        if (ctx->bonus_ship.pos.x > SCREEN_WIDTH + 10) ctx->bonus_ship.active = 0;
    }
}

void Game_Update(GameContext* ctx)
{
    update_stars(ctx);
    if (ctx->state == STATE_PLAYING) update_playing(ctx);
    update_feedback(ctx);
}

/* -------------------------------------------------------
 * Event Handling
 * ------------------------------------------------------- */

void Game_HandleEvent(GameContext* ctx, SDL_Event* e)
{
    if (e->type == SDL_QUIT) { ctx->running = 0; return; }

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key = e->key.keysym.sym;
        switch (ctx->state) {
        case STATE_MENU:
            if (key == SDLK_UP || key == SDLK_w) ctx->difficulty = (Difficulty)((ctx->difficulty + DIFFICULTY_COUNT - 1) % DIFFICULTY_COUNT);
            else if (key == SDLK_DOWN || key == SDLK_s) ctx->difficulty = (Difficulty)((ctx->difficulty + 1) % DIFFICULTY_COUNT);
            else if (key == SDLK_RETURN || key == SDLK_SPACE) reset_run(ctx);
            else if (key == SDLK_h || key == SDLK_F1) ctx->state = STATE_HIGHSCORES;
            else if (key == SDLK_a) ctx->state = STATE_ACHIEVEMENTS;
            else if (key == SDLK_o) { ctx->menu_selection = STATE_MENU; ctx->state = STATE_SETTINGS; }
            else if (key == SDLK_t) ctx->state = STATE_TUTORIAL;
            else if (key == SDLK_ESCAPE) ctx->running = 0;
            break;
        case STATE_TUTORIAL:
            if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_ESCAPE) {
                ctx->settings.show_tutorial = 0;
                ctx->state = (ctx->score == 0 && ctx->player.lives > 0) ? STATE_PLAYING : STATE_MENU;
            }
            break;
        case STATE_PLAYING:
            if (key == SDLK_ESCAPE || key == SDLK_p) ctx->state = STATE_PAUSED;
            break;
        case STATE_PAUSED:
            if (key == SDLK_ESCAPE || key == SDLK_p) ctx->state = STATE_PLAYING;
            else if (key == SDLK_q) { save_context(ctx); ctx->state = STATE_MENU; }
            else if (key == SDLK_o) { ctx->menu_selection = STATE_PAUSED; ctx->state = STATE_SETTINGS; }
            break;
        case STATE_SETTINGS:
            if (key == SDLK_ESCAPE || key == SDLK_RETURN) { save_context(ctx); ctx->state = (GameState)ctx->menu_selection; }
            else if (key == SDLK_LEFT || key == SDLK_a) ctx->settings.sfx_volume = clamp_int(ctx->settings.sfx_volume - 10, 0, 100);
            else if (key == SDLK_RIGHT || key == SDLK_d) ctx->settings.sfx_volume = clamp_int(ctx->settings.sfx_volume + 10, 0, 100);
            else if (key == SDLK_m) ctx->settings.music_volume = (ctx->settings.music_volume > 0) ? 0 : 45;
            else if (key == SDLK_s) ctx->settings.screen_shake = !ctx->settings.screen_shake;
            break;
        case STATE_GAMEOVER:
            if (ctx->name_entry_active) {
                if (key == SDLK_RETURN && ctx->input_len > 0) {
                    Scores_Insert(&ctx->scores, ctx->input_name, ctx->score, ctx->level, (int)ctx->difficulty);
                    save_context(ctx);
                    ctx->hi_score = ctx->scores.entries[0].score;
                    ctx->name_entry_active = 0;
                } else if (key == SDLK_BACKSPACE && ctx->input_len > 0) {
                    ctx->input_name[--ctx->input_len] = '\0';
                }
            } else if (key == SDLK_RETURN || key == SDLK_SPACE) ctx->state = STATE_MENU;
            break;
        case STATE_VICTORY:
            if (key == SDLK_RETURN || key == SDLK_SPACE) {
                Game_ResetPlayer(ctx);
                Game_StartLevel(ctx);
                ctx->state = STATE_PLAYING;
            }
            break;
        case STATE_HIGHSCORES:
        case STATE_ACHIEVEMENTS:
            if (key == SDLK_ESCAPE || key == SDLK_RETURN) ctx->state = STATE_MENU;
            break;
        default:
            break;
        }
    }

    if (e->type == SDL_TEXTINPUT && ctx->state == STATE_GAMEOVER && ctx->name_entry_active && ctx->input_len < MAX_NAME_LEN) {
        char c = e->text.text[0];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
            ctx->input_name[ctx->input_len++] = c;
            ctx->input_name[ctx->input_len] = '\0';
        }
    }
}

/* -------------------------------------------------------
 * Main Game Loop
 * ------------------------------------------------------- */

void Game_Run(SDL_Renderer* renderer)
{
    RendererState rs;
    if (!Renderer_Init(&rs)) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Renderer_Init failed - continuing without text");

    GameContext ctx;
    Game_InitContext(&ctx);
    SDL_StartTextInput();

    while (ctx.running) {
        ctx.frame_start = SDL_GetTicks();
        Uint32 ticks = ctx.frame_start;
        SDL_Event e;
        while (SDL_PollEvent(&e)) Game_HandleEvent(&ctx, &e);
        Game_Update(&ctx);

        SDL_SetRenderDrawColor(renderer, COLOR_BG);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        int shake_x = 0, shake_y = 0;
        if (ctx.screen_shake_timer > 0 && ctx.settings.screen_shake) {
            int s = ctx.screen_shake_strength;
            shake_x = (rand() % (s * 2 + 1)) - s;
            shake_y = (rand() % (s * 2 + 1)) - s;
        }
        SDL_Rect viewport = { shake_x, shake_y, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderSetViewport(renderer, &viewport);

        switch (ctx.state) {
            case STATE_MENU: Renderer_DrawMenu(renderer, &rs, &ctx, ticks); break;
            case STATE_PLAYING: Renderer_DrawGame(renderer, &rs, &ctx, ticks); break;
            case STATE_PAUSED: Renderer_DrawGame(renderer, &rs, &ctx, ticks); SDL_RenderSetViewport(renderer, NULL); Renderer_DrawPause(renderer, &rs, &ctx); break;
            case STATE_GAMEOVER: Renderer_DrawGameOver(renderer, &rs, &ctx, ticks); break;
            case STATE_VICTORY: Renderer_DrawVictory(renderer, &rs, &ctx, ticks); break;
            case STATE_HIGHSCORES: Renderer_DrawHighScores(renderer, &rs, &ctx, ticks); break;
            case STATE_ACHIEVEMENTS: Renderer_DrawAchievements(renderer, &rs, &ctx, ticks); break;
            case STATE_SETTINGS: Renderer_DrawSettings(renderer, &rs, &ctx, ticks); break;
            case STATE_TUTORIAL: Renderer_DrawTutorial(renderer, &rs, &ctx, ticks); break;
            default: break;
        }
        SDL_RenderSetViewport(renderer, NULL);
        SDL_RenderPresent(renderer);

        Uint32 elapsed = SDL_GetTicks() - ctx.frame_start;
        if (elapsed < FRAME_DELAY_MS) SDL_Delay(FRAME_DELAY_MS - elapsed);
    }

    save_context(&ctx);
    SDL_StopTextInput();
    if (g_audio_device) {
        SDL_CloseAudioDevice(g_audio_device);
        g_audio_device = 0;
    }
    Renderer_Destroy(&rs);
}

/* -------------------------------------------------------
 * Score Persistence
 * ------------------------------------------------------- */

static void normalize_scores(ScoreTable* table)
{
    table->count = clamp_int(table->count, 0, MAX_SCORES);
    for (int i = 0; i < table->count; i++) {
        table->entries[i].name[MAX_NAME_LEN] = '\0';
        if (!table->entries[i].name[0]) snprintf(table->entries[i].name, sizeof(table->entries[i].name), "PILOT");
        if (table->entries[i].difficulty < 0 || table->entries[i].difficulty >= DIFFICULTY_COUNT)
            table->entries[i].difficulty = DIFFICULTY_NORMAL;
    }
}

void Scores_Load(ScoreTable* table)
{
    memset(table, 0, sizeof(*table));
    FILE* f = fopen(SAVE_FILE_NAME, "rb");
    if (!f) return;

    SaveData data;
    memset(&data, 0, sizeof(data));
    size_t got = fread(&data, 1, sizeof(data), f);
    fclose(f);

    if (got == sizeof(data) && data.magic == SAVE_MAGIC && data.version == SAVE_VERSION) {
        *table = data.scores;
    } else if (got >= sizeof(int)) {
        /* Backward compatibility with the original v1 high-score-only file. */
        f = fopen(SAVE_FILE_NAME, "rb");
        if (!f) return;
        fread(&table->count, sizeof(int), 1, f);
        table->count = clamp_int(table->count, 0, MAX_SCORES);
        fread(table->entries, sizeof(ScoreEntry) - sizeof(int), (size_t)table->count, f);
        fclose(f);
    }
    normalize_scores(table);
}

void Scores_Save(const ScoreTable* table)
{
    SaveData data;
    memset(&data, 0, sizeof(data));
    data.magic = SAVE_MAGIC;
    data.version = SAVE_VERSION;
    data.scores = *table;
    FILE* f = fopen(SAVE_FILE_NAME, "wb");
    if (!f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to save %s", SAVE_FILE_NAME);
        return;
    }
    if (fwrite(&data, 1, sizeof(data), f) != sizeof(data)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to write complete save file");
    }
    fclose(f);
}

int Scores_IsHighScore(const ScoreTable* table, int score)
{
    if (score <= 0) return 0;
    if (table->count < MAX_SCORES) return 1;
    return score > table->entries[table->count - 1].score;
}

void Scores_Insert(ScoreTable* table, const char* name, int score, int level, int difficulty)
{
    int pos = table->count;
    for (int i = 0; i < table->count; i++) {
        if (score > table->entries[i].score) { pos = i; break; }
    }
    if (pos >= MAX_SCORES) return;

    int new_count = (table->count < MAX_SCORES) ? table->count + 1 : MAX_SCORES;
    for (int i = new_count - 1; i > pos; i--) table->entries[i] = table->entries[i - 1];

    snprintf(table->entries[pos].name, sizeof(table->entries[pos].name), "%s", (name && name[0]) ? name : "PILOT");
    table->entries[pos].score = score;
    table->entries[pos].level = level;
    table->entries[pos].difficulty = clamp_int(difficulty, 0, DIFFICULTY_COUNT - 1);
    table->count = new_count;
}

/* Exposed names for renderer without duplicating gameplay data. */
const char* Game_DifficultyName(Difficulty difficulty)
{
    return DIFFICULTY_CONFIGS[clamp_int((int)difficulty, 0, DIFFICULTY_COUNT - 1)].name;
}

const char* Game_AchievementName(int id)
{
    return (id >= 0 && id < ACH_COUNT) ? ACH_NAMES[id] : "UNKNOWN";
}

const char* Game_AchievementDescription(int id)
{
    return (id >= 0 && id < ACH_COUNT) ? ACH_DESC[id] : "";
}
