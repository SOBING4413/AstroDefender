/*
 * AstroDefender - Core Game Logic
 * game.c
 *
 * Handles state machine, update loop, collision detection,
 * score persistence, and enemy AI.
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "game.h"
#include "config.h"
#include "renderer.h"

/* -------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------- */

static void init_stars(GameContext* ctx)
{
    for (int layer = 0; layer < STAR_LAYERS; layer++) {
        for (int i = 0; i < STARS_PER_LAYER; i++) {
            Star* s = &ctx->stars[layer][i];
            s->x          = (float)(rand() % SCREEN_WIDTH);
            s->y          = (float)(rand() % SCREEN_HEIGHT);
            s->speed      = 0.3f + layer * 0.35f;
            s->brightness = (Uint8)(80 + layer * 60 + rand() % 60);
            s->size       = layer + 1;
        }
    }
}

static void update_stars(GameContext* ctx)
{
    for (int layer = 0; layer < STAR_LAYERS; layer++) {
        for (int i = 0; i < STARS_PER_LAYER; i++) {
            Star* s = &ctx->stars[layer][i];
            s->y += s->speed;
            if (s->y > SCREEN_HEIGHT) {
                s->y = 0.0f;
                s->x = (float)(rand() % SCREEN_WIDTH);
            }
        }
    }
}

/* -------------------------------------------------------
 * Initialization
 * ------------------------------------------------------- */

void Game_InitContext(GameContext* ctx)
{
    memset(ctx, 0, sizeof(GameContext));
    srand((unsigned int)time(NULL));

    ctx->state          = STATE_MENU;
    ctx->running        = 1;
    ctx->level          = 1;
    ctx->enemy_direction = 1;

    Scores_Load(&ctx->scores);
    if (ctx->scores.count > 0)
        ctx->hi_score = ctx->scores.entries[0].score;

    init_stars(ctx);
    Game_ResetPlayer(ctx);
}

void Game_ResetPlayer(GameContext* ctx)
{
    ctx->player.pos.x       = (float)(SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2);
    ctx->player.pos.y       = (float)(SCREEN_HEIGHT - PLAYER_HEIGHT - 24);
    ctx->player.active      = 1;
    ctx->player.invincible  = 0;
    ctx->player.last_shot_time = 0;
}

void Game_StartLevel(GameContext* ctx)
{
    /* Clear bullets */
    memset(ctx->player_bullets, 0, sizeof(ctx->player_bullets));
    memset(ctx->enemy_bullets,  0, sizeof(ctx->enemy_bullets));
    memset(ctx->particles,      0, sizeof(ctx->particles));

    /* Place enemies */
    ctx->enemies_alive = 0;
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            Enemy* e  = &ctx->enemies[row][col];
            e->pos.x  = (float)(ENEMY_START_X + col * ENEMY_PADDING_X);
            e->pos.y  = (float)(ENEMY_START_Y + row * ENEMY_PADDING_Y);
            e->alive  = 1;
            e->anim_frame = 0;
            e->anim_timer = 0;

            /* Assign type based on row */
            if      (row == 0)                e->type = ENEMY_TYPE_BOSS;
            else if (row <= 1)                e->type = ENEMY_TYPE_C;
            else if (row <= 2)                e->type = ENEMY_TYPE_B;
            else                              e->type = ENEMY_TYPE_A;

            ctx->enemies_alive++;
        }
    }

    /* Enemy movement speed increases each level */
    float speed = ENEMY_SPEED_INIT + (ctx->level - 1) * ENEMY_SPEED_STEP;
    ctx->enemy_dx            = speed;
    ctx->enemy_direction     = 1;
    ctx->enemy_move_interval = 30; /* Will decrease as enemies die */
    ctx->enemy_move_timer    = 0;

    /* Bonus ship reset */
    ctx->bonus_ship.active = 0;
    ctx->next_bonus_time   = SDL_GetTicks() + BONUS_SPAWN_INTERVAL;
}

/* -------------------------------------------------------
 * Collision
 * ------------------------------------------------------- */

int Game_RectOverlap(float ax, float ay, int aw, int ah,
                     float bx, float by, int bw, int bh)
{
    return (ax < bx + bw && ax + aw > bx &&
            ay < by + bh && ay + ah > by);
}

/* -------------------------------------------------------
 * Explosion Particles
 * ------------------------------------------------------- */

void Game_SpawnExplosion(GameContext* ctx, float x, float y,
                         Uint8 r, Uint8 g, Uint8 b, int count)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ctx->particles[i].life && count > 0) {
            Particle* p = &ctx->particles[i];
            float angle = (float)(rand() % 628) / 100.0f;
            float speed = 0.5f + (float)(rand() % 30) / 10.0f;
            p->pos.x    = x;
            p->pos.y    = y;
            p->vel.x    = cosf(angle) * speed;
            p->vel.y    = sinf(angle) * speed;
            p->life     = PARTICLE_LIFETIME - rand() % 15;
            p->max_life = p->life;
            p->r        = r;
            p->g        = g;
            p->b        = b;
            p->size     = 1.5f + (float)(rand() % 3);
            count--;
        }
    }
}

/* -------------------------------------------------------
 * Shoot helpers
 * ------------------------------------------------------- */

static void player_shoot(GameContext* ctx)
{
    Uint32 now = SDL_GetTicks();
    if (now - ctx->player.last_shot_time < PLAYER_SHOOT_DELAY) return;

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!ctx->player_bullets[i].active) {
            Bullet* b   = &ctx->player_bullets[i];
            b->pos.x    = ctx->player.pos.x + PLAYER_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
            b->pos.y    = ctx->player.pos.y - BULLET_HEIGHT;
            b->dy       = -PLAYER_BULLET_SPEED;
            b->active   = 1;
            ctx->player.last_shot_time = now;
            break;
        }
    }
}

static void enemy_try_shoot(GameContext* ctx, int row, int col)
{
    /* Only bottom-most alive enemy in column may shoot */
    for (int r = ENEMY_ROWS - 1; r > row; r--) {
        if (ctx->enemies[r][col].alive) return;
    }

    if (rand() % (ENEMY_SHOOT_CHANCE * TARGET_FPS) != 0) return;

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!ctx->enemy_bullets[i].active) {
            Enemy*  e = &ctx->enemies[row][col];
            Bullet* b = &ctx->enemy_bullets[i];
            b->pos.x  = e->pos.x + ENEMY_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
            b->pos.y  = e->pos.y + ENEMY_HEIGHT;
            b->dy     = ENEMY_BULLET_SPEED + (ctx->level - 1) * 0.3f;
            b->active = 1;
            break;
        }
    }
}

/* -------------------------------------------------------
 * Update: Playing State
 * ------------------------------------------------------- */

static void update_playing(GameContext* ctx)
{
    Uint32 now = SDL_GetTicks();

    /* Stars */
    update_stars(ctx);

    /* Keyboard state */
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    /* Player movement */
    if (ctx->player.active) {
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
            ctx->player.pos.x -= PLAYER_SPEED;
            if (ctx->player.pos.x < 0) ctx->player.pos.x = 0;
        }
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
            ctx->player.pos.x += PLAYER_SPEED;
            if (ctx->player.pos.x > SCREEN_WIDTH - PLAYER_WIDTH)
                ctx->player.pos.x = (float)(SCREEN_WIDTH - PLAYER_WIDTH);
        }
        if (keys[SDL_SCANCODE_SPACE])
            player_shoot(ctx);

        /* Expire invincibility */
        if (ctx->player.invincible &&
            now - ctx->player.hit_time > PLAYER_INVINCIBLE_MS) {
            ctx->player.invincible = 0;
        }
    }

    /* Player bullets */
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        Bullet* b = &ctx->player_bullets[i];
        if (!b->active) continue;
        b->pos.y += b->dy;
        if (b->pos.y + BULLET_HEIGHT < 0) {
            b->active = 0;
            continue;
        }

        /* Check bonus ship collision */
        if (ctx->bonus_ship.active &&
            Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                             ctx->bonus_ship.pos.x, ctx->bonus_ship.pos.y,
                             BONUS_SHIP_WIDTH, BONUS_SHIP_HEIGHT)) {
            b->active = 0;
            ctx->bonus_ship.active = 0;
            ctx->score += SCORE_BONUS_SHIP;
            if (ctx->score > ctx->hi_score) ctx->hi_score = ctx->score;
            ctx->score_flash_timer = 45;
            Game_SpawnExplosion(ctx,
                ctx->bonus_ship.pos.x + BONUS_SHIP_WIDTH / 2.0f,
                ctx->bonus_ship.pos.y + BONUS_SHIP_HEIGHT / 2.0f,
                255, 160, 40, 20);
            continue;
        }

        /* Check enemy collisions */
        for (int row = 0; row < ENEMY_ROWS; row++) {
            for (int col = 0; col < ENEMY_COLS; col++) {
                Enemy* e = &ctx->enemies[row][col];
                if (!e->alive) continue;
                if (Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                                     e->pos.x, e->pos.y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
                    b->active = 0;
                    e->alive  = 0;
                    ctx->enemies_alive--;

                    /* Score by type */
                    int pts = 0;
                    switch (e->type) {
                        case ENEMY_TYPE_A:    pts = SCORE_ENEMY_TYPE0; break;
                        case ENEMY_TYPE_B:    pts = SCORE_ENEMY_TYPE1; break;
                        case ENEMY_TYPE_C:    pts = SCORE_ENEMY_TYPE2; break;
                        case ENEMY_TYPE_BOSS: pts = SCORE_ENEMY_TYPE3; break;
                        default: break;
                    }
                    ctx->score += pts;
                    if (ctx->score > ctx->hi_score) ctx->hi_score = ctx->score;
                    ctx->score_flash_timer = 20;

                    /* Explosion color by type */
                    Uint8 er = 255, eg = 100, eb = 40;
                    if (e->type == ENEMY_TYPE_BOSS)   { er = 180; eg = 80; eb = 255; }
                    else if (e->type == ENEMY_TYPE_C) { er = 80;  eg = 255; eb = 120; }
                    else if (e->type == ENEMY_TYPE_B) { er = 0;   eg = 220; eb = 255; }
                    Game_SpawnExplosion(ctx,
                        e->pos.x + ENEMY_WIDTH / 2.0f,
                        e->pos.y + ENEMY_HEIGHT / 2.0f,
                        er, eg, eb, 16);

                    /* Speed up enemies as they thin out */
                    ctx->enemy_move_interval = 5 + (ctx->enemies_alive * 25) / (ENEMY_ROWS * ENEMY_COLS);
                    if (ctx->enemy_move_interval < 5) ctx->enemy_move_interval = 5;

                    goto bullet_done;
                }
            }
        }
        bullet_done:;
    }

    /* Enemy bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        Bullet* b = &ctx->enemy_bullets[i];
        if (!b->active) continue;
        b->pos.y += b->dy;
        if (b->pos.y > SCREEN_HEIGHT) {
            b->active = 0;
            continue;
        }

        /* Hit player */
        if (ctx->player.active && !ctx->player.invincible &&
            Game_RectOverlap(b->pos.x, b->pos.y, BULLET_WIDTH, BULLET_HEIGHT,
                             ctx->player.pos.x + 4, ctx->player.pos.y + 4,
                             PLAYER_WIDTH - 8, PLAYER_HEIGHT - 8)) {
            b->active = 0;
            ctx->player.lives--;
            ctx->player.invincible = 1;
            ctx->player.hit_time   = now;
            ctx->life_lost_flash   = 45;
            Game_SpawnExplosion(ctx,
                ctx->player.pos.x + PLAYER_WIDTH / 2.0f,
                ctx->player.pos.y + PLAYER_HEIGHT / 2.0f,
                255, 80, 80, 24);

            if (ctx->player.lives <= 0) {
                ctx->player.active = 0;
                /* Delay before game over */
                SDL_Delay(800);
                ctx->state = STATE_GAMEOVER;
                if (Scores_IsHighScore(&ctx->scores, ctx->score)) {
                    ctx->name_entry_active = 1;
                    ctx->input_len = 0;
                    memset(ctx->input_name, 0, sizeof(ctx->input_name));
                }
                return;
            }
        }
    }

    /* Enemy movement */
    ctx->enemy_move_timer++;
    if (ctx->enemy_move_timer >= ctx->enemy_move_interval) {
        ctx->enemy_move_timer = 0;

        /* Check if any enemy hits the wall */
        int hit_wall = 0;
        for (int row = 0; row < ENEMY_ROWS; row++) {
            for (int col = 0; col < ENEMY_COLS; col++) {
                Enemy* e = &ctx->enemies[row][col];
                if (!e->alive) continue;
                float nx = e->pos.x + ctx->enemy_dx * ctx->enemy_direction;
                if (nx < 8 || nx + ENEMY_WIDTH > SCREEN_WIDTH - 8)
                    hit_wall = 1;
            }
        }

        if (hit_wall) {
            ctx->enemy_direction = -ctx->enemy_direction;
            /* Drop down */
            for (int row = 0; row < ENEMY_ROWS; row++)
                for (int col = 0; col < ENEMY_COLS; col++)
                    if (ctx->enemies[row][col].alive)
                        ctx->enemies[row][col].pos.y += ENEMY_DROP_DISTANCE;
        }
        else {
            for (int row = 0; row < ENEMY_ROWS; row++)
                for (int col = 0; col < ENEMY_COLS; col++)
                    if (ctx->enemies[row][col].alive)
                        ctx->enemies[row][col].pos.x += ctx->enemy_dx * ctx->enemy_direction;
        }

        /* Animate enemies */
        for (int row = 0; row < ENEMY_ROWS; row++)
            for (int col = 0; col < ENEMY_COLS; col++)
                if (ctx->enemies[row][col].alive)
                    ctx->enemies[row][col].anim_frame ^= 1;
    }

    /* Enemy shooting */
    for (int row = 0; row < ENEMY_ROWS; row++)
        for (int col = 0; col < ENEMY_COLS; col++)
            if (ctx->enemies[row][col].alive)
                enemy_try_shoot(ctx, row, col);

    /* Check enemy reached player zone */
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            if (ctx->enemies[row][col].alive &&
                ctx->enemies[row][col].pos.y + ENEMY_HEIGHT >= ctx->player.pos.y) {
                ctx->state = STATE_GAMEOVER;
                if (Scores_IsHighScore(&ctx->scores, ctx->score)) {
                    ctx->name_entry_active = 1;
                    ctx->input_len = 0;
                    memset(ctx->input_name, 0, sizeof(ctx->input_name));
                }
                return;
            }
        }
    }

    /* Victory check */
    if (ctx->enemies_alive == 0) {
        ctx->level++;
        ctx->state = STATE_VICTORY;
        return;
    }

    /* Bonus ship */
    if (!ctx->bonus_ship.active && now >= ctx->next_bonus_time) {
        ctx->bonus_ship.active  = 1;
        ctx->bonus_ship.pos.y   = 30.0f;
        ctx->bonus_ship.dx      = BONUS_SHIP_SPEED;
        ctx->bonus_ship.pos.x   = -BONUS_SHIP_WIDTH;
        ctx->next_bonus_time    = now + BONUS_SPAWN_INTERVAL + (rand() % 10000);
    }

    if (ctx->bonus_ship.active) {
        ctx->bonus_ship.pos.x += ctx->bonus_ship.dx;
        if (ctx->bonus_ship.pos.x > SCREEN_WIDTH + 10)
            ctx->bonus_ship.active = 0;
    }

    /* Particles */
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &ctx->particles[i];
        if (!p->life) continue;
        p->pos.x += p->vel.x;
        p->pos.y += p->vel.y;
        p->vel.y += 0.05f; /* Gravity */
        p->life--;
    }

    /* Decrement flash timers */
    if (ctx->score_flash_timer > 0) ctx->score_flash_timer--;
    if (ctx->life_lost_flash   > 0) ctx->life_lost_flash--;
}

/* -------------------------------------------------------
 * Event Handling
 * ------------------------------------------------------- */

void Game_HandleEvent(GameContext* ctx, SDL_Event* e)
{
    if (e->type == SDL_QUIT) {
        ctx->running = 0;
        return;
    }

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key = e->key.keysym.sym;

        switch (ctx->state) {

        case STATE_MENU:
            if (key == SDLK_RETURN || key == SDLK_SPACE) {
                ctx->score  = 0;
                ctx->level  = 1;
                ctx->player.lives = PLAYER_MAX_LIVES;
                Game_ResetPlayer(ctx);
                Game_StartLevel(ctx);
                ctx->state = STATE_PLAYING;
            }
            if (key == SDLK_h || key == SDLK_F1)
                ctx->state = STATE_HIGHSCORES;
            if (key == SDLK_ESCAPE)
                ctx->running = 0;
            break;

        case STATE_PLAYING:
            if (key == SDLK_ESCAPE || key == SDLK_p)
                ctx->state = STATE_PAUSED;
            break;

        case STATE_PAUSED:
            if (key == SDLK_ESCAPE || key == SDLK_p)
                ctx->state = STATE_PLAYING;
            if (key == SDLK_q) {
                ctx->state = STATE_MENU;
            }
            break;

        case STATE_GAMEOVER:
            if (ctx->name_entry_active) {
                /* Name input */
                if (key == SDLK_RETURN && ctx->input_len > 0) {
                    Scores_Insert(&ctx->scores, ctx->input_name,
                                  ctx->score, ctx->level);
                    Scores_Save(&ctx->scores);
                    ctx->hi_score          = ctx->scores.entries[0].score;
                    ctx->name_entry_active = 0;
                }
                else if (key == SDLK_BACKSPACE && ctx->input_len > 0) {
                    ctx->input_name[--ctx->input_len] = '\0';
                }
            }
            else {
                if (key == SDLK_RETURN || key == SDLK_SPACE)
                    ctx->state = STATE_MENU;
            }
            break;

        case STATE_VICTORY:
            if (key == SDLK_RETURN || key == SDLK_SPACE) {
                Game_ResetPlayer(ctx);
                Game_StartLevel(ctx);
                ctx->state = STATE_PLAYING;
            }
            break;

        case STATE_HIGHSCORES:
            if (key == SDLK_ESCAPE || key == SDLK_RETURN)
                ctx->state = STATE_MENU;
            break;

        default:
            break;
        }
    }

    /* Text input for name entry */
    if (e->type == SDL_TEXTINPUT &&
        ctx->state == STATE_GAMEOVER &&
        ctx->name_entry_active &&
        ctx->input_len < MAX_NAME_LEN) {
        char c = e->text.text[0];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
             c == '_' || c == '-') {
            ctx->input_name[ctx->input_len++] = c;
            ctx->input_name[ctx->input_len]   = '\0';
        }
    }
}

/* -------------------------------------------------------
 * Main Update Dispatcher
 * ------------------------------------------------------- */

void Game_Update(GameContext* ctx)
{
    update_stars(ctx); /* Always animate background */

    if (ctx->state == STATE_PLAYING)
        update_playing(ctx);

    /* Particle decay in non-playing states too */
    if (ctx->state != STATE_PLAYING) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (ctx->particles[i].life > 0)
                ctx->particles[i].life--;
        }
    }
}

/* -------------------------------------------------------
 * Main Game Loop
 * ------------------------------------------------------- */

void Game_Run(SDL_Renderer* renderer)
{
    RendererState rs;
    if (!Renderer_Init(&rs)) {
        SDL_Log("Renderer_Init failed - continuing without text");
    }

    GameContext ctx;
    Game_InitContext(&ctx);

    SDL_StartTextInput();

    while (ctx.running) {
        ctx.frame_start = SDL_GetTicks();
        Uint32 ticks = ctx.frame_start;

        SDL_Event e;
        while (SDL_PollEvent(&e))
            Game_HandleEvent(&ctx, &e);

        Game_Update(&ctx);

        /* Full render dispatch */
        SDL_SetRenderDrawColor(renderer, COLOR_BG);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        switch (ctx.state) {
            case STATE_MENU:
                Renderer_DrawMenu(renderer, &rs, &ctx, ticks);
                break;
            case STATE_PLAYING:
                Renderer_DrawGame(renderer, &rs, &ctx, ticks);
                break;
            case STATE_PAUSED:
                Renderer_DrawGame(renderer, &rs, &ctx, ticks);
                Renderer_DrawPause(renderer, &rs, &ctx);
                break;
            case STATE_GAMEOVER:
                Renderer_DrawGameOver(renderer, &rs, &ctx, ticks);
                break;
            case STATE_VICTORY:
                Renderer_DrawVictory(renderer, &rs, &ctx, ticks);
                break;
            case STATE_HIGHSCORES:
                Renderer_DrawHighScores(renderer, &rs, &ctx, ticks);
                break;
            default:
                break;
        }

        SDL_RenderPresent(renderer);

        /* Cap frame rate */
        Uint32 elapsed = SDL_GetTicks() - ctx.frame_start;
        if (elapsed < FRAME_DELAY_MS)
            SDL_Delay(FRAME_DELAY_MS - elapsed);
    }

    SDL_StopTextInput();
    Renderer_Destroy(&rs);
}

/* -------------------------------------------------------
 * Score Persistence
 * ------------------------------------------------------- */

void Scores_Load(ScoreTable* table)
{
    table->count = 0;
    FILE* f = fopen(SAVE_FILE_NAME, "rb");
    if (!f) return;
    fread(&table->count, sizeof(int), 1, f);
    if (table->count > MAX_SCORES) table->count = MAX_SCORES;
    fread(table->entries, sizeof(ScoreEntry), (size_t)table->count, f);
    fclose(f);
}

void Scores_Save(const ScoreTable* table)
{
    FILE* f = fopen(SAVE_FILE_NAME, "wb");
    if (!f) return;
    fwrite(&table->count, sizeof(int), 1, f);
    fwrite(table->entries, sizeof(ScoreEntry), (size_t)table->count, f);
    fclose(f);
}

int Scores_IsHighScore(const ScoreTable* table, int score)
{
    if (table->count < MAX_SCORES) return 1;
    return score > table->entries[table->count - 1].score;
}

void Scores_Insert(ScoreTable* table, const char* name, int score, int level)
{
    int pos = table->count;

    for (int i = 0; i < table->count; i++)
    {
        if (score > table->entries[i].score)
        {
            pos = i;
            break;
        }
    }

    int new_count =
        (table->count < MAX_SCORES)
        ? table->count + 1
        : MAX_SCORES;

    for (int i = new_count - 1; i > pos; i--)
    {
        table->entries[i] = table->entries[i - 1];
    }

    strncpy_s(
        table->entries[pos].name,
        sizeof(table->entries[pos].name),
        name,
        _TRUNCATE
    );

    table->entries[pos].score = score;
    table->entries[pos].level = level;
    table->count = new_count;
}