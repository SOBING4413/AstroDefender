/*
 * AstroDefender - Core Game Types
 * types.h
 *
 * Shared data structures used throughout the game.
 */

#ifndef ASTRODEFENDER_TYPES_H
#define ASTRODEFENDER_TYPES_H

#include <SDL.h>
#include "config.h"

/* -------------------------------------------------------
 * Enumerations
 * ------------------------------------------------------- */

typedef enum {
    STATE_MENU = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_VICTORY,
    STATE_HIGHSCORES
} GameState;

typedef enum {
    ENEMY_TYPE_A = 0,   /* Bottom rows  - 10 pts */
    ENEMY_TYPE_B,       /* Middle rows  - 20 pts */
    ENEMY_TYPE_C,       /* Upper rows   - 30 pts */
    ENEMY_TYPE_BOSS,    /* Top row      - 50 pts */
    ENEMY_TYPE_COUNT
} EnemyType;

/* -------------------------------------------------------
 * Vectors & Geometry
 * ------------------------------------------------------- */

typedef struct {
    float x, y;
} Vec2;

/* -------------------------------------------------------
 * Player
 * ------------------------------------------------------- */

typedef struct {
    Vec2     pos;
    int      lives;
    int      active;
    Uint32   last_shot_time;
    Uint32   hit_time;          /* Time of last hit for invincibility */
    int      invincible;
} Player;

/* -------------------------------------------------------
 * Bullet
 * ------------------------------------------------------- */

typedef struct {
    Vec2  pos;
    float dy;
    int   active;
} Bullet;

/* -------------------------------------------------------
 * Enemy
 * ------------------------------------------------------- */

typedef struct {
    Vec2       pos;
    EnemyType  type;
    int        alive;
    int        anim_frame;
    int        anim_timer;
} Enemy;

/* -------------------------------------------------------
 * Bonus Ship
 * ------------------------------------------------------- */

typedef struct {
    Vec2   pos;
    float  dx;
    int    active;
    int    visible_timer;
} BonusShip;

/* -------------------------------------------------------
 * Particle
 * ------------------------------------------------------- */

typedef struct {
    Vec2  pos;
    Vec2  vel;
    int   life;
    int   max_life;
    Uint8 r, g, b;
    float size;
} Particle;

/* -------------------------------------------------------
 * Star (Background Layer)
 * ------------------------------------------------------- */

typedef struct {
    float x, y;
    float speed;
    Uint8 brightness;
    int   size;
} Star;

/* -------------------------------------------------------
 * High Score Entry
 * ------------------------------------------------------- */

#define MAX_NAME_LEN  12
#define MAX_SCORES     8

typedef struct {
    char  name[MAX_NAME_LEN + 1];
    int   score;
    int   level;
} ScoreEntry;

typedef struct {
    ScoreEntry entries[MAX_SCORES];
    int        count;
} ScoreTable;

/* -------------------------------------------------------
 * Master Game Context
 * ------------------------------------------------------- */

typedef struct {
    GameState   state;
    int         level;
    int         score;
    int         hi_score;
    int         running;

    Player      player;
    Bullet      player_bullets[MAX_PLAYER_BULLETS];
    Bullet      enemy_bullets[MAX_ENEMY_BULLETS];

    Enemy       enemies[ENEMY_ROWS][ENEMY_COLS];
    int         enemies_alive;
    float       enemy_dx;           /* Current horizontal step */
    int         enemy_move_timer;
    int         enemy_move_interval;/* Frames between steps */
    int         enemy_direction;    /* +1 right, -1 left */

    BonusShip   bonus_ship;
    Uint32      next_bonus_time;

    Particle    particles[MAX_PARTICLES];
    Star        stars[STAR_LAYERS][STARS_PER_LAYER];

    ScoreTable  scores;
    char        input_name[MAX_NAME_LEN + 1];
    int         input_len;
    int         name_entry_active;

    /* Timing */
    Uint32      frame_start;
    float       delta_time;         /* Seconds since last frame */

    /* HUD flash */
    int         score_flash_timer;
    int         life_lost_flash;
} GameContext;

#endif /* ASTRODEFENDER_TYPES_H */
