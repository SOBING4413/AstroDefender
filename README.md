# AstroDefender

**Classic Space Shooter built with C + SDL2**
**Version 1.0**

---

## Overview

**AstroDefender** is a modern reimagining of the classic *Space Invaders* arcade experience, developed entirely in **C** using **SDL2**. Defend Earth against endless waves of alien invaders, survive increasingly difficult levels, and compete for a place on the local high-score leaderboard.

### Features

* 4 unique enemy classes with procedural animated sprites
* Endless progression with increasing difficulty
* Mystery Bonus Ship worth **500 points**
* Persistent local high-score table (Top 8)
* Parallax starfield background
* Particle-based explosion effects
* Temporary invincibility after taking damage
* Pause and resume functionality
* Smooth gameplay locked at **60 FPS**

---

## System Requirements

### Operating System

* Windows 10 (64-bit) or newer
* Windows 11 (64-bit)

### Development Environment

* Visual Studio Community 2022 or later
* Workload: **Desktop Development with C++**

### Additional Requirements

* Internet connection (required only for initial dependency download)

---

## Quick Start

### 1. Install Dependencies

Run:

```bat
setup_deps.bat
```

This script automatically downloads:

* SDL2
* SDL2_ttf
* SDL2_mixer

from their official GitHub releases and places them inside the `deps/` directory.

> Initial setup typically takes 30–60 seconds depending on connection speed.

### 2. Open the Solution

Open:

```text
AstroDefender.sln
```

using Visual Studio.

### 3. Build the Project

Use one of the following:

```text
Ctrl + Shift + B
```

or:

```text
Build → Build Solution
```

Generated executable:

```text
bin/Release/AstroDefender.exe
```

### 4. Run the Game

```text
F5
```

Run with debugger

or

```text
Ctrl + F5
```

Run without debugger

You can also launch the executable directly from:

```text
bin/Release/
```

---

## Project Structure

```text
AstroDefender/
│
├── AstroDefender.sln
├── setup_deps.bat
├── README.md
│
├── src/
│   ├── main.c
│   ├── game.c
│   └── renderer.c
│
├── include/
│   ├── config.h
│   ├── types.h
│   ├── game.h
│   └── renderer.h
│
├── AstroDefender/
│   └── AstroDefender.vcxproj
│
├── resources/
│   ├── resource.rc
│   └── icon.ico
│
├── assets/
│   └── fonts/
│       └── PressStart2P.ttf (optional)
│
└── deps/
    ├── SDL2/
    ├── SDL2_ttf/
    └── SDL2_mixer/
```

### Directory Details

| Directory    | Purpose                                        |
| ------------ | ---------------------------------------------- |
| `src/`       | Core game logic and rendering                  |
| `include/`   | Shared headers and configuration               |
| `resources/` | Windows resources and application icon         |
| `assets/`    | Optional fonts and future game assets          |
| `deps/`      | Third-party libraries downloaded automatically |

---

## Controls

| Key     | Action                           |
| ------- | -------------------------------- |
| ← / A   | Move Left                        |
| → / D   | Move Right                       |
| Space   | Fire Weapon                      |
| P / Esc | Pause / Resume                   |
| Q       | Quit to Main Menu (while paused) |
| H       | View High Scores                 |
| Enter   | Confirm / Continue               |

---

## Enemy Types & Scoring

| Enemy      | Description                 | Score |
| ---------- | --------------------------- | ----- |
| Crawler    | Green, lower formation rows | 10    |
| Crab       | Blue, middle rows           | 20    |
| Drone      | Cyan, upper rows            | 30    |
| Commander  | Purple, top row             | 50    |
| Bonus Ship | Orange mystery ship         | 500   |

---

## Gameplay Notes

* Enemy movement accelerates as their numbers decrease.
* Every new level increases enemy speed and projectile speed.
* The game ends immediately if an invader reaches the player's altitude.
* Players begin with **3 lives**.
* After taking damage, the ship becomes invulnerable for **2 seconds**.
* High scores are saved automatically to:

```text
astrodefender.sav
```

located beside the executable.

---

## Optional Retro Font

For an authentic arcade appearance, install the **Press Start 2P** font.

Download:

https://fonts.google.com/specimen/Press+Start+2P

Place the file as:

```text
assets/fonts/PressStart2P.ttf
```

The game loads it automatically if available.

If not found, AstroDefender falls back to:

* Consolas
* Lucida Console

---

## Configuration

All gameplay tuning parameters are centralized in:

```text
include/config.h
```

Examples:

* Screen resolution
* Enemy movement speed
* Difficulty scaling
* Scoring values
* Gameplay timing

After modifying values, rebuild the project.

---

## Build Configurations

### Debug

* Debug symbols enabled
* Console window visible
* Ideal for development and troubleshooting

### Release

* Optimized compiler settings
* No console window
* Windows subsystem enabled

Both configurations automatically copy required SDL DLLs to the output directory using the project's Post-Build Event.

---

## Credits

Developed using:

* SDL2
* SDL2_ttf
* SDL2_mixer
* Visual Studio 2022

Inspired by the golden age of arcade space shooters.

---

**Defend Earth. Survive the invasion. Chase the high score.**
