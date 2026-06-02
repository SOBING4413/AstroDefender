/*
 * AstroDefender - Renderer Module Interface
 * renderer.h
 */

#ifndef ASTRODEFENDER_RENDERER_H
#define ASTRODEFENDER_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "types.h"

/* Renderer state (fonts, cached surfaces) */
typedef struct {
    TTF_Font* font_large;   /* Title / large headings */
    TTF_Font* font_medium;  /* HUD labels */
    TTF_Font* font_small;   /* Small text */
    int       initialized;
} RendererState;

/* Lifecycle */
int  Renderer_Init(RendererState* rs);
void Renderer_Destroy(RendererState* rs);

/* Scene rendering */
void Renderer_DrawMenu(SDL_Renderer* r, const RendererState* rs,
                       const GameContext* ctx, Uint32 ticks);
void Renderer_DrawGame(SDL_Renderer* r, const RendererState* rs,
                       const GameContext* ctx, Uint32 ticks);
void Renderer_DrawPause(SDL_Renderer* r, const RendererState* rs,
                        const GameContext* ctx);
void Renderer_DrawGameOver(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks);
void Renderer_DrawVictory(SDL_Renderer* r, const RendererState* rs,
                          const GameContext* ctx, Uint32 ticks);
void Renderer_DrawHighScores(SDL_Renderer* r, const RendererState* rs,
                             const GameContext* ctx, Uint32 ticks);
void Renderer_DrawAchievements(SDL_Renderer* r, const RendererState* rs,
                               const GameContext* ctx, Uint32 ticks);
void Renderer_DrawSettings(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks);
void Renderer_DrawTutorial(SDL_Renderer* r, const RendererState* rs,
                           const GameContext* ctx, Uint32 ticks);
void Renderer_DrawOnline(SDL_Renderer* r, const RendererState* rs,
                         const GameContext* ctx, Uint32 ticks);

/* Primitive helpers */
void Renderer_DrawText(SDL_Renderer* r, TTF_Font* font,
                       const char* text, int x, int y,
                       Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca,
                       int centered);

void Renderer_DrawFilledRect(SDL_Renderer* r,
                             int x, int y, int w, int h,
                             Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca);

void Renderer_DrawBorderedRect(SDL_Renderer* r,
                               int x, int y, int w, int h,
                               Uint8 fr, Uint8 fg, Uint8 fb,
                               Uint8 br, Uint8 bg, Uint8 bb);

/* Game object drawers */
void Renderer_DrawStars(SDL_Renderer* r, const GameContext* ctx);
void Renderer_DrawPlayer(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks);
void Renderer_DrawEnemies(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks);
void Renderer_DrawBullets(SDL_Renderer* r, const GameContext* ctx);
void Renderer_DrawBonusShip(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks);
void Renderer_DrawParticles(SDL_Renderer* r, const GameContext* ctx);
void Renderer_DrawPowerUps(SDL_Renderer* r, const GameContext* ctx, Uint32 ticks);
void Renderer_DrawFloatingText(SDL_Renderer* r, const RendererState* rs, const GameContext* ctx);
void Renderer_DrawHUD(SDL_Renderer* r, const RendererState* rs,
                      const GameContext* ctx, Uint32 ticks);

#endif /* ASTRODEFENDER_RENDERER_H */
