/*
 * AstroDefender - Game Module Interface
 * game.h
 */

#ifndef ASTRODEFENDER_GAME_H
#define ASTRODEFENDER_GAME_H

#include <SDL.h>
#include "types.h"

/* Main entry point called from main.c */
void Game_Run(SDL_Renderer* renderer);

/* State initializers */
void Game_InitContext(GameContext* ctx);
void Game_StartLevel(GameContext* ctx);
void Game_ResetPlayer(GameContext* ctx);

/* Per-frame updates */
void Game_Update(GameContext* ctx);
void Game_HandleEvent(GameContext* ctx, SDL_Event* e);

/* Rendering */
void Game_Render(SDL_Renderer* renderer, GameContext* ctx);

/* Utility */
void Game_SpawnExplosion(GameContext* ctx, float x, float y,
                         Uint8 r, Uint8 g, Uint8 b, int count);
int  Game_RectOverlap(float ax, float ay, int aw, int ah,
                      float bx, float by, int bw, int bh);

/* Audio feedback */
void Game_PlayTone(int frequency, int duration_ms, int volume);
const char* Game_DifficultyName(Difficulty difficulty);
const char* Game_AchievementName(int id);
const char* Game_AchievementDescription(int id);

/* Persistence */
void Scores_Load(ScoreTable* table);
void Scores_Save(const ScoreTable* table);
int  Scores_IsHighScore(const ScoreTable* table, int score);
void Scores_Insert(ScoreTable* table, const char* name, int score, int level, int difficulty);

#endif /* ASTRODEFENDER_GAME_H */
