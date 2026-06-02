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
    Renderer_DrawText(r, NULL, NULL, 0, 0, 0, 0, 0, 0, 0); /* No-op: bonus text in HUD */
}

/* -------------------------------------------------------
 * Particles
 * ------------------------------------------------------- */
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
    Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH-100, 12, 180, 220, 255, 255, 0);

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
    Renderer_DrawText(r, rs->font_small, "ARROWS/AD:Move  SPACE:Fire  P:Pause",
                      SCREEN_WIDTH/2, SCREEN_HEIGHT-24, 80, 110, 140, 200, 1);

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
                      SCREEN_WIDTH/2, 290, 120, 160, 200, 200, 1);

    dline(r, SCREEN_WIDTH/2-160, 316, SCREEN_WIDTH/2+160, 316, 0, 80, 140);

    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_medium, "PRESS ENTER TO START",
                          SCREEN_WIDTH/2, 360, 0, 220, 255, 255, 1);

    Renderer_DrawText(r, rs->font_small, "[H] HIGH SCORES",  SCREEN_WIDTH/2, 420, 100, 140, 180, 200, 1);
    Renderer_DrawText(r, rs->font_small, "[ESC] QUIT",        SCREEN_WIDTH/2, 450, 100, 140, 180, 200, 1);

    /* Enemy legend */
    int lx = SCREEN_WIDTH/2 - 100, ly = 502;
    Renderer_DrawText(r, rs->font_small, "CRAWLER  = 10 pts", lx, ly,    80, 255, 120, 220, 0);
    Renderer_DrawText(r, rs->font_small, "CRAB     = 20 pts", lx, ly+24, 0,  220, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "DRONE    = 30 pts", lx, ly+48, 80, 200, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "COMMANDER= 50 pts", lx, ly+72, 180, 80, 255, 220, 0);
    Renderer_DrawText(r, rs->font_small, "BONUS SHIP= 500 pts", lx, ly+96, 255, 160, 40, 220, 0);

    Renderer_DrawText(r, rs->font_small, "v1.0", SCREEN_WIDTH-40, SCREEN_HEIGHT-20, 60, 80, 100, 180, 0);
}

void Renderer_DrawGame(SDL_Renderer* r, const RendererState* rs,
                       const GameContext* ctx, Uint32 ticks) {
    Renderer_DrawStars(r, ctx);
    Renderer_DrawEnemies(r, ctx, ticks);
    Renderer_DrawBonusShip(r, ctx, ticks);
    Renderer_DrawBullets(r, ctx);
    Renderer_DrawPlayer(r, ctx, ticks);
    Renderer_DrawParticles(r, ctx);
    Renderer_DrawHUD(r, rs, ctx, ticks);
}

void Renderer_DrawPause(SDL_Renderer* r, const RendererState* rs, const GameContext* ctx) {
    (void)ctx;
    Renderer_DrawFilledRect(r, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 4, 18, 190);
    Renderer_DrawBorderedRect(r, SCREEN_WIDTH/2-180, SCREEN_HEIGHT/2-90, 360, 180, 0, 18, 50, 0, 100, 160);
    Renderer_DrawText(r, rs->font_large, "PAUSED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2-70, 0, 220, 255, 255, 1);
    Renderer_DrawText(r, rs->font_small, "[P] RESUME",       SCREEN_WIDTH/2, SCREEN_HEIGHT/2+10, 180, 220, 255, 220, 1);
    Renderer_DrawText(r, rs->font_small, "[Q] QUIT TO MENU", SCREEN_WIDTH/2, SCREEN_HEIGHT/2+38, 180, 220, 255, 220, 1);
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
            snprintf(buf, sizeof(buf), "%d.  %-12s  %07d  LVL %02d",
                     i+1, e->name, e->score, e->level);
            Renderer_DrawText(r, rs->font_medium, buf, SCREEN_WIDTH/2, 152+i*44, cr, cg, cb, 255, 1);
        }
    }

    if ((ticks/500)%2 == 0)
        Renderer_DrawText(r, rs->font_small, "PRESS ENTER OR ESC TO RETURN",
                          SCREEN_WIDTH/2, SCREEN_HEIGHT-60, 80, 120, 160, 200, 1);
}
