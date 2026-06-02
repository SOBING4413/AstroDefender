=============================================================
  ASTRODEFENDER
  Classic Space Shooter - Windows / Visual Studio
  Version 1.0
=============================================================

CONCEPT
-------
AstroDefender is a faithful reimagining of the classic space
invader genre, built entirely in C using SDL2. Wave after wave
of alien invaders descend from above. You are the last line of
defense. Survive, score, and climb the leaderboard.

The game features:
  - 4 distinct enemy types with animated procedural sprites
  - Progressive difficulty across unlimited levels
  - Mystery bonus ship worth 500 points
  - Persistent local high score table (top 8 entries)
  - Parallax star field background
  - Particle explosion effects
  - Invincibility frames after being hit
  - Pause / resume support
  - Runs at a locked 60 FPS


REQUIREMENTS
------------
  - Windows 10 or 11 (64-bit)
  - Visual Studio Community 2022 (or newer)
    Install workload: "Desktop development with C++"
  - Internet connection (for first-time dependency setup)


QUICK START
-----------
1. Run setup_deps.bat
   Double-click it. It downloads SDL2, SDL2_ttf, and SDL2_mixer
   from their official GitHub releases into the deps\ folder.
   This takes about 30-60 seconds.

2. Open the solution
   Double-click AstroDefender.sln in Visual Studio.

3. Build
   Press Ctrl+Shift+B  (or Build > Build Solution).
   Output: bin\Release\AstroDefender.exe

4. Run
   Press F5 (Debug) or Ctrl+F5 (Run without debugger).
   The .exe is also runnable directly from bin\Release\.


PROJECT STRUCTURE
-----------------
AstroDefender\
  AstroDefender.sln          Visual Studio solution
  setup_deps.bat             Dependency downloader (run first)
  README.txt                 This file
  |
  +-- src\
  |     main.c               Entry point, SDL2 init/teardown
  |     game.c               Game logic, state machine, AI, physics
  |     renderer.c           All drawing code (procedural sprites)
  |
  +-- include\
  |     config.h             All tunable constants in one place
  |     types.h              Shared data structures
  |     game.h               Game module interface
  |     renderer.h           Renderer module interface
  |
  +-- AstroDefender\
  |     AstroDefender.vcxproj  VS project file
  |
  +-- resources\
  |     resource.rc          Windows resource (icon, version info)
  |     icon.ico             Application icon (16/32/48 px)
  |
  +-- assets\
  |     fonts\               Optional: place PressStart2P.ttf here
  |                          (falls back to system Consolas if absent)
  |
  +-- deps\                  Created by setup_deps.bat
        SDL2\
        SDL2_ttf\
        SDL2_mixer\


CONTROLS
--------
  Left Arrow / A     Move ship left
  Right Arrow / D    Move ship right
  Space              Fire
  P / Escape         Pause / Resume
  Q (while paused)   Quit to main menu
  H (main menu)      View high scores
  Enter              Confirm / advance screens


ENEMY TYPES & SCORING
---------------------
  Crawler   (green,  bottom rows)  -  10 points
  Crab      (blue,   mid rows)     -  20 points
  Drone     (cyan,   upper rows)   -  30 points
  Commander (purple, top row)      -  50 points
  Bonus Ship (orange, crosses top) - 500 points


GAMEPLAY NOTES
--------------
- Enemies speed up significantly as their numbers decrease.
- Difficulty increases each level (faster movement, faster bullets).
- If any enemy reaches your ship's altitude, the game ends.
- You have 3 lives. After a hit, you are invincible for 2 seconds.
- High scores are saved to astrodefender.sav next to the .exe.


OPTIONAL: PIXEL FONT
--------------------
For the authentic retro look, download "Press Start 2P" from:
  https://fonts.google.com/specimen/Press+Start+2P

Place PressStart2P-Regular.ttf renamed to PressStart2P.ttf in:
  assets\fonts\PressStart2P.ttf

The game will pick it up automatically. Without it, it uses
Consolas (or Lucida Console) from your Windows installation.


CONFIGURATION
-------------
All game parameters (speed, difficulty, scoring, screen size)
are centralized in include\config.h. Recompile after changes.


BUILD CONFIGURATION
-------------------
  Debug   - Includes debug symbols, console window visible
  Release - Optimized, no console window (SubSystem=Windows)

Both configurations copy required DLLs to the output folder
automatically via the PostBuildEvent in the project file.


=============================================================
