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
    STATE_HIGHSCORES,
    STATE_ACHIEVEMENTS,
    STATE_SETTINGS,
    STATE_TUTORIAL,
    STATE_ONLINE
} GameState;

typedef enum {
    MODE_ARCADE = 0,
    MODE_STORY,
    MODE_SURVIVAL,
    MODE_BOSS_RUSH,
    MODE_ONLINE,
    MODE_COUNT
} GameMode;

typedef enum {
    DISPLAY_WINDOWED = 0,
    DISPLAY_FULLSCREEN,
    DISPLAY_BORDERLESS,
    DISPLAY_MINIMIZED,
    DISPLAY_COUNT
} DisplayMode;

typedef enum {
    DIFFICULTY_EASY = 0,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD,
    DIFFICULTY_NIGHTMARE,
    DIFFICULTY_COUNT
} Difficulty;

typedef enum {
    ENEMY_TYPE_A = 0,   /* Bottom rows  - 10 pts */
    ENEMY_TYPE_B,       /* Middle rows  - 20 pts */
    ENEMY_TYPE_C,       /* Upper rows   - 30 pts */
    ENEMY_TYPE_BOSS,    /* Top row      - 50 pts */
    ENEMY_TYPE_COUNT
} EnemyType;

typedef enum {
    POWERUP_NONE = 0,
    POWERUP_SHIELD,
    POWERUP_RAPID_FIRE,
    POWERUP_DOUBLE_SHOT,
    POWERUP_REPAIR,
    POWERUP_COUNT
} PowerUpType;

typedef enum {
    EVENT_NONE = 0,
    EVENT_METEOR_STORM,
    EVENT_SCORE_SURGE,
    EVENT_SHIELD_DRIFT,
    EVENT_COUNT
} RandomEventType;

typedef enum {
    ACH_FIRST_BLOOD = 0,
    ACH_COMBO_MASTER,
    ACH_WAVE_RIDER,
    ACH_BONUS_HUNTER,
    ACH_NIGHTMARE,
    ACH_COUNT
} AchievementId;

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
    int      shield_active;
    Uint32   shield_until;
    Uint32   rapid_until;
    Uint32   double_shot_until;
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
 * Bonus Ship / Pickups / Feedback
 * ------------------------------------------------------- */

typedef struct {
    Vec2   pos;
    float  dx;
    int    active;
    int    visible_timer;
} BonusShip;

typedef struct {
    Vec2        pos;
    float       dy;
    int         active;
    PowerUpType type;
    int         ttl;
} PowerUp;

typedef struct {
    Vec2  pos;
    int   active;
    int   life;
    char  text[32];
    Uint8 r, g, b;
} FloatingText;

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
 * Save Data
 * ------------------------------------------------------- */

#define MAX_NAME_LEN  12
#define MAX_SCORES     8

typedef struct {
    char  name[MAX_NAME_LEN + 1];
    int   score;
    int   level;
    int   difficulty;
} ScoreEntry;

typedef struct {
    ScoreEntry entries[MAX_SCORES];
    int        count;
} ScoreTable;

typedef struct {
    int games_played;
    int enemies_destroyed;
    int bonus_destroyed;
    int waves_cleared;
    int best_combo;
    int total_score;
    int daily_seed;
    int daily_best;
} PlayerStats;

typedef struct {
    int unlocked[ACH_COUNT];
    int reward_claimed[ACH_COUNT];
} AchievementState;

typedef struct {
    int music_volume;
    int sfx_volume;
    int screen_shake;
    int show_tutorial;
    int display_mode;
    int resolution_index;
    int window_width;
    int window_height;
} Settings;

/* -------------------------------------------------------
 * Master Game Context
 * ------------------------------------------------------- */

typedef struct {
    GameState   state;
    int         level;
    int         score;
    int         hi_score;
    int         running;
    Difficulty  difficulty;
    GameMode    game_mode;
    int         menu_selection;

    Player      player;
    Bullet      player_bullets[MAX_PLAYER_BULLETS];
    Bullet      enemy_bullets[MAX_ENEMY_BULLETS];
    PowerUp     powerups[MAX_POWERUPS];
    FloatingText floating_texts[MAX_FLOATING_TEXTS];

    Enemy       enemies[ENEMY_ROWS][ENEMY_COLS];
    int         enemies_alive;
    float       enemy_dx;           /* Current horizontal step */
    int         enemy_move_timer;
    int         enemy_move_interval;/* Frames between steps */
    int         enemy_direction;    /* +1 right, -1 left */

    BonusShip   bonus_ship;
    Uint32      next_bonus_time;

    RandomEventType active_event;
    Uint32      event_until;
    Uint32      next_event_time;

    Particle    particles[MAX_PARTICLES];
    Star        stars[STAR_LAYERS][STARS_PER_LAYER];

    ScoreTable  scores;
    PlayerStats stats;
    AchievementState achievements;
    Settings    settings;
    char        online_email[64];
    int         online_input_active;
    int         online_logged_in;
    int         online_sync_pending;
    char        online_status[128];
    char        input_name[MAX_NAME_LEN + 1];
    int         input_len;
    int         name_entry_active;

    /* Progression / rewards */
    int         combo;
    int         best_combo_this_run;
    Uint32      last_kill_time;
    int         score_multiplier;
    int         milestone_next_score;
    int         last_reward_points;
    int         story_chapter;
    int         story_objective;

    /* Timing */
    Uint32      frame_start;
    float       delta_time;         /* Seconds since last frame */

    /* HUD flash / effects */
    int         score_flash_timer;
    int         life_lost_flash;
    int         screen_shake_timer;
    int         screen_shake_strength;
    char        status_message[64];
    int         status_timer;
} GameContext;

#endif /* ASTRODEFENDER_TYPES_H */
