/*
 * AstroDefender - Renderer
 * renderer.c
 *
 * All visual output. Procedurally generated sprites, no external images needed.
 * Rendering is dispatched from Game_Run which owns RendererState.
 */

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "renderer.h"
#include "config.h"
#include "game.h"

/* -------------------------------------------------------
 * Font loading - tries bundled then system fonts
 * ------------------------------------------------------- */
static const char* FONT_PATHS[] = {
    "assets/fonts/PressStart2P.ttf",
    "C:/Windows/Fonts/consola.ttf",
    "C:/Windows/Fonts/cour.ttf",
    "C:/Windows/Fonts/lucon.ttf",
    NULL
};

static TTF_Font* load_best_font(int size) {
    for (int i = 0; FONT_PATHS[i]; i++) {
        TTF_Font* f = TTF_OpenFont(FONT_PATHS[i], size);
        if (f) return f;
    }
    return NULL;
}

int Renderer_Init(RendererState* rs) {
    memset(rs, 0, sizeof(RendererState));
    rs->font_large  = load_best_font(26);
    rs->font_medium = load_best_font(15);
    rs->font_small  = load_best_font(11);
    rs->initialized = 1;
    return 1;
}

void Renderer_Destroy(RendererState* rs) {
    if (rs->font_large)  TTF_CloseFont(rs->font_large);
    if (rs->font_medium) TTF_CloseFont(rs->font_medium);
    if (rs->font_small)  TTF_CloseFont(rs->font_small);
}

/* -------------------------------------------------------
 * Primitive helpers
 * ------------------------------------------------------- */

void Renderer_DrawText(SDL_Renderer* r, TTF_Font* font,
                       const char* text, int x, int y,
                       Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca, int centered) {
    if (!font || !text || !text[0]) return;
    SDL_Color col = {cr, cg, cb, ca};
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = { centered ? x - surf->w/2 : x, y, surf->w, surf->h };
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void Renderer_DrawFilledRect(SDL_Renderer* r, int x, int y, int w, int h,
                              Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, cr, cg, cb, ca);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void Renderer_DrawBorderedRect(SDL_Renderer* r, int x, int y, int w, int h,
                                Uint8 fr, Uint8 fg, Uint8 fb,
                                Uint8 br, Uint8 bg, Uint8 bb) {
    Renderer_DrawFilledRect(r, x, y, w, h, fr, fg, fb, 200);
    SDL_SetRenderDrawColor(r, br, bg, bb, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(r, &rect);
}

static void dline(SDL_Renderer* r, int x1, int y1, int x2, int y2,
                  Uint8 cr, Uint8 cg, Uint8 cb) {
    SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
}

/* -------------------------------------------------------
 * Stars
 * ------------------------------------------------------- */
void Renderer_DrawStars(SDL_Renderer* r, const GameContext* ctx) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int l = 0; l < STAR_LAYERS; l++) {
        for (int i = 0; i < STARS_PER_LAYER; i++) {
            const Star* s = &ctx->stars[l][i];
            Uint8 b = s->brightness;
            Uint8 bl = (b + 30 > 255) ? 255 : b + 30;
            SDL_SetRenderDrawColor(r, b, b, bl, 255);
            SDL_Rect rect = {(int)s->x, (int)s->y, s->size, s->size};
            SDL_RenderFillRect(r, &rect);
        }
    }
}

/* -------------------------------------------------------
 * Player
 * ------------------------------------------------------- */
void Renderer_DrawPlayer(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks) {
    if (!ctx->player.active) return;
    if (ctx->player.invincible && (ticks / 80) % 2 == 0) return;

    int x = (int)ctx->player.pos.x, y = (int)ctx->player.pos.y;
    int w = PLAYER_WIDTH, h = PLAYER_HEIGHT;

    /* Thruster */
    int fh = 6 + (ticks / 60) % 5;
    Renderer_DrawFilledRect(r, x+w/2-2, y+h, 4, fh,   255, 255, 200, 220);
    Renderer_DrawFilledRect(r, x+w/2-4, y+h, 8, fh-2, 0,   200, 255, 160);

    /* Body fill */
    for (int row = y+6; row < y+h-8; row++) {
        int hw = (int)((float)(h-8-(row-y-6))/(float)(h-14)*9.0f);
        if (hw < 1) hw = 1;
        Renderer_DrawFilledRect(r, x+w/2-hw, row, hw*2, 1, 20, 100, 180, 200);
    }
    /* Outline */
    SDL_SetRenderDrawColor(r, 40, 200, 255, 255);
    SDL_Point nose[] = {{x+w/2,y+3},{x+w/2+9,y+h/2},{x+w/2+5,y+h-8},{x+w/2-5,y+h-8},{x+w/2-9,y+h/2}};
    SDL_RenderDrawLines(r, nose, 5);
    SDL_RenderDrawLine(r, nose[4].x, nose[4].y, nose[0].x, nose[0].y);

    /* Wings */
    SDL_SetRenderDrawColor(r, 0, 220, 255, 255);
    SDL_RenderDrawLine(r, x+w/2-7,y+h/2,   x+2,       y+h-8);
    SDL_RenderDrawLine(r, x+2,    y+h-8,   x+w/2-5,   y+h-8);
    SDL_RenderDrawLine(r, x+w/2+7,y+h/2,   x+w-2,     y+h-8);
    SDL_RenderDrawLine(r, x+w-2,  y+h-8,   x+w/2+5,   y+h-8);

    /* Cockpit */
    Renderer_DrawFilledRect(r, x+w/2-3, y+7, 6, 9, 80, 255, 220, 210);
}

/* -------------------------------------------------------
 * Enemies
 * ------------------------------------------------------- */
static void draw_enemy(SDL_Renderer* r, const Enemy* e, Uint32 ticks) {
    int x = (int)e->pos.x, y = (int)e->pos.y;
    int w = ENEMY_WIDTH, h = ENEMY_HEIGHT;
    int af = e->anim_frame;

    switch (e->type) {
    case ENEMY_TYPE_A: {
        /* Green crawler */
        Renderer_DrawFilledRect(r, x+8, y+6, w-16, h-14, 40, 180, 80, 190);
        SDL_SetRenderDrawColor(r, 80, 255, 120, 255);
        SDL_Rect b = {x+8, y+6, w-16, h-14}; SDL_RenderDrawRect(r, &b);
        Renderer_DrawFilledRect(r, x+10, y+10, 5, 5, 255, 255, 80, 240);
        Renderer_DrawFilledRect(r, x+w-15, y+10, 5, 5, 255, 255, 80, 240);
        for (int t = 0; t < 4; t++) {
            dline(r, x+8+t*7, y+h-8, x+5+t*7, y+h-2+af*4, 80, 255, 120);
        }
        break;
    }
    case ENEMY_TYPE_B: {
        /* Blue crab */
        Renderer_DrawFilledRect(r, x+6, y+7, w-12, h-14, 20, 120, 200, 185);
        SDL_SetRenderDrawColor(r, 0, 220, 255, 255);
        SDL_Rect b = {x+6, y+7, w-12, h-14}; SDL_RenderDrawRect(r, &b);
        int co = af ? -3 : 3;
        dline(r, x+7, y+12, x+2+co, y+7, 0, 220, 255);
        dline(r, x+w-7, y+12, x+w-2-co, y+7, 0, 220, 255);
        Renderer_DrawFilledRect(r, x+10, y+11, w-20, 5, 0, 255, 220, 220);
        break;
    }
    case ENEMY_TYPE_C: {
        /* Cyan drone */
        Renderer_DrawFilledRect(r, x+10, y+9, w-20, h-18, 20, 80, 200, 180);
        SDL_SetRenderDrawColor(r, 80, 200, 255, 255);
        SDL_Rect b = {x+10, y+9, w-20, h-18}; SDL_RenderDrawRect(r, &b);
        if (af) {
            dline(r, x+10, y+12, x+2, y+18, 80, 200, 255);
            dline(r, x+w-10, y+12, x+w-2, y+18, 80, 200, 255);
        } else {
            dline(r, x+10, y+16, x+2, y+12, 80, 200, 255);
            dline(r, x+w-10, y+16, x+w-2, y+12, 80, 200, 255);
        }
        Renderer_DrawFilledRect(r, x+w/2-4, y+h/2-4, 8, 8, 100, 200, 255, 200);
        break;
    }
    case ENEMY_TYPE_BOSS: {
        /* Purple commander */
        Renderer_DrawFilledRect(r, x+4, y+4, w-8, h-8, 100, 20, 180, 200);
        SDL_SetRenderDrawColor(r, 180, 80, 255, 255);
        SDL_Rect b = {x+4, y+4, w-8, h-8}; SDL_RenderDrawRect(r, &b);
        for (int s = 0; s < 3; s++) {
            int sx = x+8+s*12;
            dline(r, sx, y+4, sx+4, y-4+af*2, 200, 100, 255);
        }
        Renderer_DrawFilledRect(r, x+10, y+12, 6, 6, 255, 60, 255, 240);
        Renderer_DrawFilledRect(r, x+w-16, y+12, 6, 6, 255, 60, 255, 240);
        Uint8 pulse = (Uint8)(140 + (int)(sinf(ticks*0.008f)*80));
        Renderer_DrawFilledRect(r, x+w/2-4, y+h/2-3, 8, 8, pulse, 40, 255, 220);
        break;
    }
    default: break;
    }
}

void Renderer_DrawEnemies(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks) {
    for (int row = 0; row < ENEMY_ROWS; row++)
        for (int col = 0; col < ENEMY_COLS; col++)
            if (ctx->enemies[row][col].alive)
                draw_enemy(r, &ctx->enemies[row][col], ticks);
}

/* -------------------------------------------------------
 * Bullets
 * ------------------------------------------------------- */
void Renderer_DrawBullets(SDL_Renderer* r, const GameContext* ctx) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        const Bullet* b = &ctx->player_bullets[i];
        if (!b->active) continue;
        Renderer_DrawFilledRect(r, (int)b->pos.x-1, (int)b->pos.y, BULLET_WIDTH+2, BULLET_HEIGHT, 0, 200, 255, 80);
        Renderer_DrawFilledRect(r, (int)b->pos.x, (int)b->pos.y, BULLET_WIDTH, BULLET_HEIGHT, 0, 240, 255, 255);
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        const Bullet* b = &ctx->enemy_bullets[i];
        if (!b->active) continue;
        Renderer_DrawFilledRect(r, (int)b->pos.x-1, (int)b->pos.y, BULLET_WIDTH+2, BULLET_HEIGHT, 255, 60, 20, 80);
        Renderer_DrawFilledRect(r, (int)b->pos.x, (int)b->pos.y, BULLET_WIDTH, BULLET_HEIGHT, 255, 80, 40, 255);
    }
}

/* -------------------------------------------------------
 * Bonus Ship
 * ------------------------------------------------------- */
void Renderer_DrawBonusShip(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks) {
    if (!ctx->bonus_ship.active) return;
    int x = (int)ctx->bonus_ship.pos.x, y = (int)ctx->bonus_ship.pos.y;
    int w = BONUS_SHIP_WIDTH, h = BONUS_SHIP_HEIGHT;
    Uint8 pulse = (Uint8)(160 + (int)(sinf(ticks*0.015f)*80));
    Renderer_DrawFilledRect(r, x+6, y+4, w-12, h-8, 255, 120, 20, 220);
    Renderer_DrawFilledRect(r, x+16, y, w-32, 8, 255, 180, 80, 200);
    SDL_SetRenderDrawColor(r, pulse, 140, 20, 255);
    SDL_Rect b = {x+6, y+4, w-12, h-8}; SDL_RenderDrawRect(r, &b);
    for (int l = 0; l < 4; l++) {
        Uint8 on = ((ticks/120+l)%4 == 0) ? 255 : 60;
        Renderer_DrawFilledRect(r, x+8+l*10, y+h-7, 5, 4, on, on/2, 0, 220);
    }
}

/* -------------------------------------------------------
 * Particles
 * ------------------------------------------------------- */
void Renderer_DrawPowerUps(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < MAX_POWERUPS; i++) {
        const PowerUp* p = &ctx->powerups[i];
        if (!p->active) continue;
        int x = (int)p->pos.x, y = (int)p->pos.y;
        Uint8 cr = 80, cg = 255, cb = 220;
        if (p->type == POWERUP_RAPID_FIRE) { cr = 255; cg = 220; cb = 60; }
        else if (p->type == POWERUP_DOUBLE_SHOT) { cr = 180; cg = 80; cb = 255; }
        else if (p->type == POWERUP_REPAIR) { cr = 80; cg = 255; cb = 120; }
        Uint8 pulse = (Uint8)(120 + (ticks / 4) % 100);
        Renderer_DrawFilledRect(r, x-11, y-11, 22, 22, cr, cg, cb, 70);
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_Rect box = {x-8, y-8, 16, 16};
        SDL_RenderDrawRect(r, &box);
        Renderer_DrawFilledRect(r, x-3, y-3, 6, 6, pulse, 255, 255, 230);
    }
}

void Renderer_DrawParticles(SDL_Renderer* r, const GameContext* ctx) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle* p = &ctx->particles[i];
        if (!p->life) continue;
        float alpha = (float)p->life / (float)p->max_life;
        int sz = (int)(p->size * alpha + 0.5f);
        if (sz < 1) sz = 1;
        SDL_SetRenderDrawColor(r, p->r, p->g, p->b, (Uint8)(alpha * 230));
        SDL_Rect rect = {(int)p->pos.x - sz/2, (int)p->pos.y - sz/2, sz, sz};
        SDL_RenderFillRect(r, &rect);
    }
}

/* -------------------------------------------------------
 * HUD
 * ------------------------------------------------------- */
void Renderer_DrawFloatingText(SDL_Renderer* r, const RendererState* rs, const GameContext* ctx) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        const FloatingText* ft = &ctx->floating_texts[i];
        if (!ft->active) continue;
        Uint8 alpha = (Uint8)((ft->life > 60 ? 60 : ft->life) * 4);
        Renderer_DrawText(r, rs->font_small, ft->text, (int)ft->pos.x, (int)ft->pos.y, ft->r, ft->g, ft->b, alpha, 0);
    }
}

void Renderer_DrawHUD(SDL_Renderer* r, const RendererState* rs,
                      const GameContext* ctx, Uint32 ticks) {
    char buf[80];

    /* Top bar */
    Renderer_DrawFilledRect(r, 0, 0, SCREEN_WIDTH, 42, 0, 8, 24, 230);
    dline(r, 0, 42, SCREEN_WIDTH, 42, 0, 60, 120);

    /* Score */
    Uint8 sc_r = 180, sc_g = 220, sc_b = 255;
    if (ctx->score_flash_timer > 0 && (ticks/60)%2 == 0) { sc_r=255; sc_g=255; sc_b=80; }
    snprintf(buf, sizeof(buf), "SCORE %07d", ctx->score);
    Renderer_DrawText(r, rs->font_medium, buf, 16, 12, sc_r, sc_g, sc_b, 255, 0);

    snprintf(buf, sizeof(buf), "BEST  %07d", ctx->hi_score);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 12, 120, 160, 200, 255, 1);

    snprintf(buf, sizeof(buf), "LVL %02d", ctx->level);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH-128, 12, 180, 220, 255, 255, 0);

    snprintf(buf, sizeof(buf), "%s / %s", Game_ModeName(ctx->game_mode), Game_DifficultyName(ctx->difficulty));
    Renderer_DrawText(r, rs->font_small, buf, SCREEN_WIDTH-210, 30, 255, 190, 80, 230, 0);

    /* Lives */
    for (int i = 0; i < ctx->player.lives; i++) {
        int lx = 16 + i*22, ly = SCREEN_HEIGHT-26;
        SDL_SetRenderDrawColor(r, 0, 200, 255, 200);
        SDL_RenderDrawLine(r, lx+7, ly,    lx+14, ly+14);
        SDL_RenderDrawLine(r, lx+14, ly+14, lx,   ly+14);
        SDL_RenderDrawLine(r, lx,   ly+14, lx+7, ly);
    }

    /* Bottom bar */
    Renderer_DrawFilledRect(r, 0, SCREEN_HEIGHT-36, SCREEN_WIDTH, 36, 0, 8, 24, 210);
    dline(r, 0, SCREEN_HEIGHT-36, SCREEN_WIDTH, SCREEN_HEIGHT-36, 0, 60, 100);
    snprintf(buf, sizeof(buf), "COMBO x%d   %s", ctx->combo, ctx->status_timer > 0 ? ctx->status_message : "ARROWS/AD MOVE  SPACE FIRE  P PAUSE");
    Renderer_DrawText(r, rs->font_small, buf, SCREEN_WIDTH/2, SCREEN_HEIGHT-24, 80, 220, 255, 220, 1);

    int bx = SCREEN_WIDTH - 250;
    if (ctx->player.shield_active) Renderer_DrawText(r, rs->font_small, "SHIELD", bx, SCREEN_HEIGHT-24, 80, 220, 255, 230, 0);
    if (SDL_GetTicks() < ctx->player.rapid_until) Renderer_DrawText(r, rs->font_small, "RAPID", bx+70, SCREEN_HEIGHT-24, 255, 220, 60, 230, 0);
    if (SDL_GetTicks() < ctx->player.double_shot_until) Renderer_DrawText(r, rs->font_small, "DOUBLE", bx+135, SCREEN_HEIGHT-24, 180, 80, 255, 230, 0);

    if (ctx->life_lost_flash > 0) {
        Renderer_DrawFilledRect(r, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                                180, 20, 20, (Uint8)(ctx->life_lost_flash * 4));
    }
}

/* -------------------------------------------------------
 * Scene renderers
 * ------------------------------------------------------- */

void Renderer_DrawMenu(SDL_Renderer* r, const RendererState* rs,
                       const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);

    float glow = sinf(ticks * 0.003f) * 0.5f + 0.5f;
    Uint8 tgr = (Uint8)(80 + glow * 120), tgg = (Uint8)(190 + glow * 60), tgb = 255;

    Renderer_DrawText(r, rs->font_large, "ASTRO",    SCREEN_WIDTH/2, 180, tgr, tgg, tgb, 255, 1);
    Renderer_DrawText(r, rs->font_large, "DEFENDER", SCREEN_WIDTH/2, 224, tgr, tgg, tgb, 255, 1);
    Renderer_DrawText(r, rs->font_small, "DEFEND EARTH FROM THE ALIEN INVASION",
                      SCREEN_WIDTH/2, 286, 120, 160, 200, 200, 1);

    dline(r, SCREEN_WIDTH/2-160, 316, SCREEN_WIDTH/2+160, 316, 0, 80, 140);

    Renderer_DrawText(r, rs->font_medium, "SELECT MODE", SCREEN_WIDTH/2, 330, 180, 220, 255, 255, 1);
    const char* modes[] = {"1 ARCADE", "2 STORY", "3 SURVIVAL", "4 BOSS", "5 ONLINE"};
    for (int m = 0; m < MODE_COUNT; m++) {
        Uint8 mr = ((int)ctx->game_mode == m) ? 255 : 100;
        Uint8 mg = ((int)ctx->game_mode == m) ? 220 : 140;
        Uint8 mb = ((int)ctx->game_mode == m) ? 60 : 180;
        Renderer_DrawText(r, rs->font_small, modes[m], SCREEN_WIDTH/2 - 220 + m * 92, 362, mr, mg, mb, 255, 0);
    }

    Renderer_DrawText(r, rs->font_medium, "SELECT DIFFICULTY", SCREEN_WIDTH/2, 398, 180, 220, 255, 255, 1);
    const char* names[] = {"EASY", "NORMAL", "HARD", "NIGHTMARE"};
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        Uint8 cr = ((int)ctx->difficulty == i) ? 255 : 100;
        Uint8 cg = ((int)ctx->difficulty == i) ? 220 : 140;
        Uint8 cb = ((int)ctx->difficulty == i) ? 60 : 180;
        char dbuf[32];
        snprintf(dbuf, sizeof(dbuf), "%s%s", ((int)ctx->difficulty == i) ? "> " : "  ", names[i]);
        Renderer_DrawText(r, rs->font_small, dbuf, SCREEN_WIDTH/2 - 90 + i * 70, 438, cr, cg, cb, 255, 0);
    }
    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_medium, ctx->game_mode == MODE_ONLINE ? "ENTER ONLINE LOBBY" : "PRESS ENTER TO LAUNCH", SCREEN_WIDTH/2, 480, 0, 220, 255, 255, 1);

    Renderer_DrawText(r, rs->font_small, "[LEFT/RIGHT] MODE  [UP/DOWN] DIFFICULTY  [H] SCORES  [A] ACH  [O] SETTINGS",
                      SCREEN_WIDTH/2, 515, 100, 140, 180, 220, 1);
    char daily[96];
    snprintf(daily, sizeof(daily), "DAILY BEST %07d   DISPLAY %s", ctx->stats.daily_best, Game_DisplayModeName((DisplayMode)ctx->settings.display_mode));
    Renderer_DrawText(r, rs->font_small, daily, SCREEN_WIDTH/2, 540, 255, 190, 80, 220, 1);
    Renderer_DrawText(r, rs->font_small, "[T] TUTORIAL  [ESC] QUIT", SCREEN_WIDTH/2, 565, 100, 140, 180, 200, 1);

    /* Enemy legend */
    int lx = SCREEN_WIDTH/2 - 100, ly = 600;
    Renderer_DrawText(r, rs->font_small, "CRAWLER  = 10 pts", lx, ly,    80, 255, 120, 220, 0);
    Renderer_DrawText(r, rs->font_small, "CRAB     = 20 pts", lx, ly+24, 0,  220, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "DRONE    = 30 pts", lx, ly+48, 80, 200, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "COMMANDER= 50 pts", lx, ly+72, 180, 80, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "BONUS SHIP= 500 pts", lx, ly+96, 255, 160, 40, 220, 0);

    Renderer_DrawText(r, rs->font_small, "v2.0 POLISHED", SCREEN_WIDTH-150, SCREEN_HEIGHT-20, 60, 80, 100, 180, 0);
}

void Renderer_DrawGame(SDL_Renderer* r, const RendererState* rs,
                       const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawEnemies(r, ctx, ticks);
    Renderer_DrawBonusShip(r, ctx, ticks);
    Renderer_DrawPowerUps(r, ctx, ticks);
    Renderer_DrawBullets(r, ctx);
    Renderer_DrawPlayer(r, ctx, ticks);
    Renderer_DrawParticles(r, ctx);
    Renderer_DrawFloatingText(r, rs, ctx);
    Renderer_DrawHUD(r, rs, ctx, ticks);
}

void Renderer_DrawPause(SDL_Renderer* r, const RendererState* rs, const GameContext* ctx) {
    (void)ctx;
    Renderer_DrawFilledRect(r, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 4, 18, 190);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-180, SCREEN_HEIGHT/2-90, 360, 180, 0, 18, 50, 0, 100, 160);
    Renderer_DrawText(r, rs->font_large, "PAUSED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2-70, 0, 220, 255, 255, 1);
    Renderer_DrawText(r, rs->font_small, "[P] RESUME",       SCREEN_WIDTH/2, SCREEN_HEIGHT/2+10, 180, 220, 255, 220, 1);
    Renderer_DrawText(r, rs->font_small, "[Q] QUIT TO MENU", SCREEN_WIDTH/2, SCREEN_HEIGHT/2+38, 180, 220, 255, 220, 1);
    Renderer_DrawText(r, rs->font_small, "[O] SETTINGS", SCREEN_WIDTH/2, SCREEN_HEIGHT/2+66, 180, 220, 255, 220, 1);
}

void Renderer_DrawGameOver(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawParticles(r, ctx);
    float pulse = sinf(ticks*0.005f)*0.5f+0.5f;
    Uint8 pr = (Uint8)(200+pulse*55);
    Renderer_DrawText(r, rs->font_large, "GAME OVER", SCREEN_WIDTH/2, 200, pr, 60, 60, 255, 1);
    char buf[64];
    snprintf(buf, sizeof(buf), "FINAL SCORE: %d", ctx->score);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 290, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "LEVEL REACHED: %d", ctx->level);
    Renderer_DrawText(r, rs->font_small, buf, SCREEN_WIDTH/2, 330, 140, 180, 220, 220, 1);

    if (ctx->name_entry_active) {
        Renderer_DrawText(r, rs->font_medium, "NEW HIGH SCORE!", SCREEN_WIDTH/2, 390, 255, 220, 0, 255, 1);
        Renderer_DrawText(r, rs->font_small,  "ENTER YOUR NAME:", SCREEN_WIDTH/2, 432, 180, 220, 255, 200, 1);
        Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-110, 458, 220, 36, 0, 18, 50, 0, 180, 255);
        snprintf(buf, sizeof(buf), "%s_", ctx->input_name);
        Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 464, 0, 255, 220, 255, 1);
        Renderer_DrawText(r, rs->font_small, "[ENTER] CONFIRM", SCREEN_WIDTH/2, 510, 100, 140, 180, 200, 1);
    } else {
        if ((ticks/500)%2 == 0)
            Renderer_DrawText(r, rs->font_medium, "PRESS ENTER TO CONTINUE",
                              SCREEN_WIDTH/2, 430, 0, 220, 255, 255, 1);
    }
}

void Renderer_DrawVictory(SDL_Renderer* r, const RendererState* rs,
                          const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawParticles(r, ctx);
    float p = sinf(ticks*0.004f)*0.5f+0.5f;
    Uint8 vg = (Uint8)(180+p*75);
    Renderer_DrawText(r, rs->font_large, "WAVE CLEARED!", SCREEN_WIDTH/2, 200, 80, vg, 100, 255, 1);
    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE: %d", ctx->score);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 290, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "ADVANCING TO LEVEL %d", ctx->level);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 330, 0, 220, 255, 220, 1);
    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_medium, "PRESS ENTER TO CONTINUE",
                          SCREEN_WIDTH/2, 430, 0, 220, 255, 255, 1);
}

void Renderer_DrawHighScores(SDL_Renderer* r, const RendererState* rs,
                             const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawText(r, rs->font_large, "HIGH SCORES", SCREEN_WIDTH/2, 80, 0, 220, 255, 255, 1);
    dline(r, SCREEN_WIDTH/2-200, 132, SCREEN_WIDTH/2+200, 132, 0, 80, 120);

    if (ctx->scores.count == 0) {
        Renderer_DrawText(r, rs->font_medium, "NO SCORES YET",
                          SCREEN_WIDTH/2, 280, 100, 140, 180, 200, 1);
    } else {
        char buf[80];
        for (int i = 0; i < ctx->scores.count; i++) {
            const ScoreEntry* e = &ctx->scores.entries[i];
            Uint8 cr=140, cg=180, cb=220;
            if (i==0) { cr=255; cg=220; cb=0; }
            else if (i==1) { cr=200; cg=200; cb=200; }
            else if (i==2) { cr=200; cg=140; cb=80; }
            snprintf(buf, sizeof(buf), "%d.  %-12s  %07d  LVL %02d  %s",
                     i+1, e->name, e->score, e->level, Game_DifficultyName((Difficulty)e->difficulty));
            Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 152+i*44, cr, cg, cb, 255, 1);
        }
    }

    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_small, "PRESS ENTER OR ESC TO RETURN",
                          SCREEN_WIDTH/2, SCREEN_HEIGHT-60, 80, 120, 160, 200, 1);
}

void Renderer_DrawAchievements(SDL_Renderer* r, const RendererState* rs,
                               const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawText(r, rs->font_large, "ACHIEVEMENTS", SCREEN_WIDTH/2, 70, 0, 220, 255, 255, 1);
    dline(r, SCREEN_WIDTH/2-230, 122, SCREEN_WIDTH/2+230, 122, 0, 80, 120);

    for (int i = 0; i < ACH_COUNT; i++) {
        int y = 160 + i * 72;
        int unlocked = ctx->achievements.unlocked[i];
        Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-310, y-12, 620, 48,
                                  unlocked ? 12 : 4, unlocked ? 40 : 16, unlocked ? 48 : 28,
                                  unlocked ? 0 : 70, unlocked ? 220 : 90, unlocked ? 255 : 120);
        Renderer_DrawText(r, rs->font_medium, Game_AchievementName(i), SCREEN_WIDTH/2-285, y,
                          unlocked ? 255 : 90, unlocked ? 220 : 120, unlocked ? 80 : 150, 255, 0);
        Renderer_DrawText(r, rs->font_small, Game_AchievementDescription(i), SCREEN_WIDTH/2-285, y+24,
                          unlocked ? 180 : 80, unlocked ? 220 : 110, unlocked ? 255 : 140, 220, 0);
        Renderer_DrawText(r, rs->font_small, unlocked ? "UNLOCKED" : "LOCKED", SCREEN_WIDTH/2+210, y+12,
                          unlocked ? 80 : 140, unlocked ? 255 : 140, unlocked ? 120 : 140, 230, 0);
    }

    char stat[128];
    snprintf(stat, sizeof(stat), "GAMES %d  KILLS %d  WAVES %d  BEST COMBO %d  BONUS %d",
             ctx->stats.games_played, ctx->stats.enemies_destroyed, ctx->stats.waves_cleared,
             ctx->stats.best_combo, ctx->stats.bonus_destroyed);
    Renderer_DrawText(r, rs->font_small, stat, SCREEN_WIDTH/2, SCREEN_HEIGHT-92, 120, 180, 220, 220, 1);
    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_small, "PRESS ENTER OR ESC TO RETURN", SCREEN_WIDTH/2, SCREEN_HEIGHT-60, 80, 120, 160, 200, 1);
}

void Renderer_DrawSettings(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks) {
    (void)ticks;
    Renderer_DrawStars(r, ctx);
    Renderer_DrawText(r, rs->font_large, "SETTINGS", SCREEN_WIDTH/2, 100, 0, 220, 255, 255, 1);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-310, 160, 620, 350, 0, 18, 50, 0, 100, 160);

    char buf[96];
    snprintf(buf, sizeof(buf), "SFX VOLUME: %3d%%   [LEFT/RIGHT]", ctx->settings.sfx_volume);
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 230, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "MUSIC: %s   [M]", ctx->settings.music_volume > 0 ? "ON" : "OFF");
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 280, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "SCREEN SHAKE: %s   [S]", ctx->settings.screen_shake ? "ON" : "OFF");
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 330, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "DISPLAY: %s   [F/F11]", Game_DisplayModeName((DisplayMode)ctx->settings.display_mode));
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 380, 180, 220, 255, 255, 1);
    snprintf(buf, sizeof(buf), "SIZE: %dx%d   [R] PRESET  [Z/X] WIDTH  [C/V] HEIGHT", ctx->settings.window_width, ctx->settings.window_height);
    Renderer_DrawText(r, rs->font_small, buf, SCREEN_WIDTH/2, 430, 180, 220, 255, 255, 1);
    Renderer_DrawText(r, rs->font_small, "Window is resizable; fullscreen/windowed/borderless/minimize are applied immediately.", SCREEN_WIDTH/2, 468, 100, 140, 180, 220, 1);
    Renderer_DrawText(r, rs->font_small, "ENTER/ESC: SAVE AND RETURN", SCREEN_WIDTH/2, 492, 100, 140, 180, 220, 1);
}

void Renderer_DrawTutorial(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks) {
    (void)ctx;
    Renderer_DrawStars(r, ctx);
    Renderer_DrawText(r, rs->font_large, "PILOT BRIEFING", SCREEN_WIDTH/2, 82, 0, 220, 255, 255, 1);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-345, 145, 690, 390, 0, 18, 50, 0, 100, 160);
    Renderer_DrawText(r, rs->font_small, "MOVE with ARROWS or A/D. Hold SPACE to fire.", SCREEN_WIDTH/2, 185, 180, 220, 255, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Destroy invaders quickly to build combos and bonus score.", SCREEN_WIDTH/2, 225, 180, 220, 255, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Collect power-ups: Shield, Rapid Fire, Double Shot, and Repair.", SCREEN_WIDTH/2, 265, 180, 220, 255, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Random events can boost scores, increase danger, or rain rewards.", SCREEN_WIDTH/2, 305, 180, 220, 255, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Harder difficulties raise speed, enemy fire, event pressure, and rewards.", SCREEN_WIDTH/2, 345, 255, 210, 80, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Pause anytime with P or ESC.", SCREEN_WIDTH/2, 385, 180, 220, 255, 230, 1);
    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_medium, "PRESS ENTER TO START", SCREEN_WIDTH/2, 475, 0, 220, 255, 255, 1);
}


void Renderer_DrawOnline(SDL_Renderer* r, const RendererState* rs,
                         const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawText(r, rs->font_large, "ONLINE HUB", SCREEN_WIDTH/2, 82, 0, 220, 255, 255, 1);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-350, 145, 700, 420, 0, 18, 50, 0, 100, 160);

    Renderer_DrawText(r, rs->font_medium, "SUPABASE LOGIN", SCREEN_WIDTH/2, 185, 255, 220, 80, 255, 1);
    Renderer_DrawText(r, rs->font_small, "Set ASTRO_SUPABASE_URL and ASTRO_SUPABASE_ANON_KEY to enable real sync plumbing.",
                      SCREEN_WIDTH/2, 225, 150, 190, 230, 220, 1);

    Renderer_DrawText(r, rs->font_small, "EMAIL:", SCREEN_WIDTH/2-270, 280, 180, 220, 255, 230, 0);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-190, 266, 390, 38, 0, 10, 35, 0, 160, 220);
    char email[96];
    snprintf(email, sizeof(email), "%s%s", ctx->online_email, ctx->online_input_active && ((ticks/400)%2==0) ? "_" : "");
    Renderer_DrawText(r, rs->font_small, email[0] ? email : "press E to type email", SCREEN_WIDTH/2-174, 278,
                      email[0] ? 0 : 90, email[0] ? 255 : 130, email[0] ? 220 : 160, 255, 0);

    Renderer_DrawText(r, rs->font_small, ctx->online_logged_in ? "STATUS: LOGGED IN" : "STATUS: OFFLINE / LOCAL ACCOUNT",
                      SCREEN_WIDTH/2, 335, ctx->online_logged_in ? 80 : 255, ctx->online_logged_in ? 255 : 180,
                      ctx->online_logged_in ? 120 : 80, 240, 1);
    Renderer_DrawText(r, rs->font_small, ctx->online_status, SCREEN_WIDTH/2, 370, 180, 220, 255, 220, 1);

    Renderer_DrawText(r, rs->font_small, "[E] EDIT EMAIL   [ENTER/L] LOGIN   [S] SYNC SCORE   [ENTER after login] START ONLINE RUN",
                      SCREEN_WIDTH/2, 438, 100, 180, 220, 230, 1);
    Renderer_DrawText(r, rs->font_small, "Online mode currently prepares Supabase auth/leaderboard payloads without blocking gameplay.",
                      SCREEN_WIDTH/2, 474, 255, 210, 80, 220, 1);
    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_small, "ESC: RETURN TO MENU", SCREEN_WIDTH/2, 522, 80, 120, 160, 220, 1);
}
