/*
 * AstroDefender - Configuration Constants
 * config.h
 *
 * Central location for all tunable game parameters.
 * Modify these values to adjust gameplay feel and difficulty.
 */

#ifndef ASTRODEFENDER_CONFIG_H
#define ASTRODEFENDER_CONFIG_H

/* -------------------------------------------------------
 * Window & Rendering
 * ------------------------------------------------------- */
#define WINDOW_TITLE        "AstroDefender"
#define SCREEN_WIDTH        960
#define SCREEN_HEIGHT       720
#define TARGET_FPS          60
#define FRAME_DELAY_MS      (1000 / TARGET_FPS)

/* Polyglot integration contract shared by C/C++/C#/Java/Python/TypeScript/Rust/Go/Lua modules.
 * ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
 */
#define ASTRO_POLYGLOT_CONTRACT_VERSION "1.0.0"
#define ASTRO_POLYGLOT_ASPECT_POLICY    "fit-letterbox"

/* -------------------------------------------------------
 * Player
 * ------------------------------------------------------- */
#define PLAYER_WIDTH        48
#define PLAYER_HEIGHT       48
#define PLAYER_SPEED        5.0f
#define PLAYER_BULLET_SPEED 9.0f
#define PLAYER_MAX_LIVES    3
#define PLAYER_SHOOT_DELAY  180   /* Milliseconds between shots */
#define PLAYER_INVINCIBLE_MS 2000 /* Invincibility duration after hit */

/* -------------------------------------------------------
 * Enemies
 * ------------------------------------------------------- */
#define ENEMY_WIDTH         40
#define ENEMY_HEIGHT        40
#define ENEMY_ROWS          4
#define ENEMY_COLS          10
#define ENEMY_START_X       60
#define ENEMY_START_Y       80
#define ENEMY_PADDING_X     72
#define ENEMY_PADDING_Y     58
#define ENEMY_SPEED_INIT    0.6f
#define ENEMY_SPEED_STEP    0.12f /* Speed increase per level */
#define ENEMY_DROP_DISTANCE 24
#define ENEMY_SHOOT_CHANCE  4    /* 1-in-N chance per frame per enemy */
#define ENEMY_BULLET_SPEED  4.5f

/* -------------------------------------------------------
 * Bullets
 * ------------------------------------------------------- */
#define BULLET_WIDTH        5
#define BULLET_HEIGHT       18
#define MAX_PLAYER_BULLETS  12
#define MAX_ENEMY_BULLETS   32
#define MAX_POWERUPS        6
#define MAX_FLOATING_TEXTS  12

/* -------------------------------------------------------
 * Scoring
 * ------------------------------------------------------- */
#define SCORE_ENEMY_TYPE0   10
#define SCORE_ENEMY_TYPE1   20
#define SCORE_ENEMY_TYPE2   30
#define SCORE_ENEMY_TYPE3   50
#define SCORE_BONUS_SHIP    500
#define SCORE_COMBO_STEP     25
#define SCORE_MILESTONE      5000

/* -------------------------------------------------------
 * Bonus / Mystery Ship
 * ------------------------------------------------------- */
#define BONUS_SHIP_WIDTH    52
#define BONUS_SHIP_HEIGHT   24
#define BONUS_SHIP_SPEED    2.8f
#define BONUS_SPAWN_INTERVAL 15000 /* Ms between possible spawns */
#define POWERUP_FALL_SPEED   2.2f
#define POWERUP_DURATION_MS  8000
#define COMBO_WINDOW_MS      1400
#define RANDOM_EVENT_MIN_MS  18000
#define RANDOM_EVENT_MAX_MS  32000
#define RANDOM_EVENT_DURATION_MS 9000

/* -------------------------------------------------------
 * Particles
 * ------------------------------------------------------- */
#define MAX_PARTICLES       128
#define PARTICLE_LIFETIME   45   /* Frames */

/* -------------------------------------------------------
 * Stars (Parallax Background)
 * ------------------------------------------------------- */
#define STAR_LAYERS         3
#define STARS_PER_LAYER     60

/* -------------------------------------------------------
 * Save File
 * ------------------------------------------------------- */
#define SAVE_FILE_NAME      "astrodefender.sav"
#define SAVE_MAGIC          0x41534432u /* ASD2 */
#define SAVE_VERSION        3
#define SUPABASE_URL_ENV    "ASTRO_SUPABASE_URL"
#define SUPABASE_KEY_ENV    "ASTRO_SUPABASE_ANON_KEY"

/* -------------------------------------------------------
 * Colors (R, G, B, A)
 * ------------------------------------------------------- */
#define COLOR_BG            0,   4,  18, 255
#define COLOR_WHITE         255, 255, 255, 255
#define COLOR_ACCENT_CYAN   0,   220, 255, 255
#define COLOR_ACCENT_GREEN  80,  255, 120, 255
#define COLOR_ACCENT_ORANGE 255, 160, 40,  255
#define COLOR_ACCENT_RED    255, 60,  60,  255
#define COLOR_ACCENT_PURPLE 180, 80,  255, 255
#define COLOR_SHIELD        100, 200, 255, 160
#define COLOR_HUD           180, 220, 255, 255
#define COLOR_DIM           100, 130, 160, 255

#endif /* ASTRODEFENDER_CONFIG_H */
