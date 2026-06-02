# AstroDefender Multi-language Examples

Folder ini berisi contoh kecil untuk menjaga aspect ratio 960x720 di berbagai bahasa dan framework game populer. Semua contoh memakai pola yang sama dengan runtime utama AstroDefender:

- render gameplay ke logical canvas/framebuffer tetap;
- hitung skala proporsional berdasarkan ukuran layar;
- present logical canvas ke viewport tengah;
- biarkan sisa area menjadi letterbox/pillarbox, bukan stretch.

Contoh yang tersedia:

- `cpp/aspect_fit.cpp` — SDL2 C++.
- `csharp/AspectFit.cs` — MonoGame/FNA style.
- `java/AspectFit.java` — libGDX `FitViewport`.
- `python/aspect_fit.py` — pygame-ce.
- `javascript-typescript/aspect_fit.ts` — Canvas/Phaser-style helper.
- `rust/aspect_fit.rs` — helper netral untuk Bevy/macroquad/ggez.
- `go/aspect_fit.go` — helper untuk Ebiten.
- `lua/aspect_fit.lua` — LÖVE canvas scaling.

Gunakan `docs/MULTILANGUAGE_GAMEDEV.md` untuk panduan struktur proyek, framework, dan integrasi antar bahasa.
