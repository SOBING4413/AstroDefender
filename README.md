# AstroDefender

**Classic Space Shooter built with C + SDL2**
**Version 2.0**

---

## Overview

**AstroDefender** is a modern reimagining of the classic *Space Invaders* arcade experience, developed entirely in **C** using **SDL2**. Defend Earth against endless waves of alien invaders, survive increasingly difficult levels, and compete for a place on the local high-score leaderboard.

### Features

* 4 selectable difficulty levels: **Easy**, **Normal**, **Hard**, and **Nightmare**
* Game modes: **Arcade**, **Story**, **Survival**, **Boss Rush**, and **Online Hub**
* Endless wave progression with scaling enemy speed, enemy fire rate, and rewards
* Combo scoring, score milestones, floating score feedback, and achievement rewards
* Power-ups: Shield, Rapid Fire, Double Shot, and Repair
* Random in-run events such as Meteor Storm, Score Surge, and Power Drift
* Mystery Bonus Ship worth high-value rewards and guaranteed power-up drops
* Persistent local high-score table (Top 8), player statistics, daily challenge best score, achievements, display settings, and online login state
* Modernized menu, HUD, display/settings, achievement, tutorial, pause, online, and scoreboard screens
* Parallax starfield, particle explosions, glow feedback, and optional screen shake
* Lightweight procedural sound feedback with configurable SFX volume
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
| A       | View Achievements (main menu)    |
| O       | Open Settings                    |
| T       | View Tutorial                    |
| ← / →   | Select Game Mode (main menu)     |
| ↑ / ↓   | Select Difficulty (main menu)    |
| F/F11   | Cycle/toggle display mode        |
| R       | Cycle resolution preset          |
| Z/X     | Decrease/increase window width   |
| C/V     | Decrease/increase window height  |
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
* Difficulty changes player lives, enemy speed, enemy fire frequency, projectile speed, event pressure, and score rewards.
* Game modes change run framing: Story tracks chapter objectives, Survival grants more endurance, Boss Rush starts harder, and Online prepares leaderboard sync.
* Quick consecutive kills extend the combo window and add bonus points.
* Every score milestone grants bonus points and drops a power-up.
* Random events periodically alter the arena: score rewards may surge, the background may accelerate during meteor pressure, or power-up drops may become more common.
* The game ends immediately if an invader reaches the player's altitude.
* After taking damage, the ship becomes invulnerable for **2 seconds** unless a shield absorbs the hit.
* High scores, statistics, achievements, display preferences, online profile email, and settings are saved automatically to:

```text
astrodefender.sav
```

located beside the executable.

---


## Display & Online Setup

The Settings screen supports windowed, exclusive fullscreen, borderless fullscreen, minimized, resizable windows, resolution presets, and keyboard-based custom width/height adjustments. The renderer now draws gameplay to a fixed 960x720 logical framebuffer and presents it through an aspect-fit viewport, so fullscreen and unusual monitor sizes use letterbox/pillarbox margins instead of stretching or flattening the game.

Online mode is Supabase-ready. Set these environment variables before launching to provide project configuration:

```text
ASTRO_SUPABASE_URL=https://your-project.supabase.co
ASTRO_SUPABASE_ANON_KEY=your-anon-key
```

The current C/SDL build stores login state locally and prepares/logs Supabase auth and leaderboard payloads without blocking gameplay; adding a concrete HTTP transport such as WinHTTP/libcurl can use the existing online status and sync hooks.

---

## Multi-language Development

AstroDefender is no longer documented as a C-only development path. The primary runtime remains C/SDL2, but the repository includes multi-language examples for common game-development ecosystems:

- C++ with SDL2-style render targets
- C# with MonoGame/FNA-style destination rectangles
- Java with libGDX `FitViewport`
- Python with pygame-ce logical surfaces
- JavaScript/TypeScript with Canvas/Phaser-style presentation
- Rust helpers suitable for Bevy, macroquad, ggez, or SDL2 bindings
- Go helpers suitable for Ebiten
- Lua helpers suitable for LÖVE

See `docs/MULTILANGUAGE_GAMEDEV.md` for recommended project structure, framework choices, and FFI/scripting integration guidance. See `examples/multilanguage/` for minimal aspect-ratio-safe snippets in each supported language.

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


## New in Version 2.0

* Added difficulty and game-mode selection before launch with Easy, Normal, Hard, Nightmare, Arcade, Story, Survival, Boss Rush, and Online balancing.
* Added progression systems: combos, milestones, achievements, daily challenge best tracking, persistent player statistics, and reward feedback.
* Added collectible power-ups and random events for replay variety.
* Added tutorial, display settings, achievements, online hub, and enhanced high-score UI screens.
* Reworked persistence with a versioned save format and backward compatibility for older score files.
* Improved reliability by removing blocking delays during gameplay updates and adding SDL logging around save/audio failures.

## Future Roadmap

* Add a concrete Supabase HTTP transport for cloud login, leaderboard reads/writes, and remote daily missions.
* Add authored background music tracks and richer layered sound effects.
* Add boss waves, daily challenge rule presets, and named local mission objectives.
* Add gamepad support and configurable key bindings.
* Add sprite/texture asset packs while keeping procedural rendering as a fallback.
