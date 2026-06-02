# Multi-language Game Development Guide

AstroDefender tetap memakai build utama C/SDL2, tetapi desainnya sekarang dapat diperlakukan sebagai **logical gameplay core** berukuran 960x720 yang dipresentasikan ke layar dengan viewport proporsional. Pendekatan yang sama dapat dipakai di bahasa lain: render dunia ke kanvas tetap, lalu salin ke backbuffer/window menggunakan letterbox atau pillarbox agar aspect ratio tidak berubah.

## Struktur proyek yang direkomendasikan

```text
AstroDefender/
├─ src/                         # C/SDL2 runtime utama
├─ include/                     # Konstanta, tipe, dan API internal C
├─ docs/MULTILANGUAGE_GAMEDEV.md
└─ examples/multilanguage/
   ├─ cpp/                      # C++ + SDL2/SDL3, raylib, atau SFML
   ├─ csharp/                   # C# + MonoGame, FNA, Unity, atau Godot C#
   ├─ java/                     # Java + libGDX
   ├─ python/                   # Python + pygame-ce atau arcade
   ├─ javascript-typescript/    # TypeScript + Phaser, PixiJS, atau Three.js
   ├─ rust/                     # Rust + Bevy, macroquad, SDL2 bindings
   ├─ go/                       # Go + Ebiten
   └─ lua/                      # Lua + LÖVE
```

## Pola fullscreen yang harus dipakai semua bahasa

1. Simpan ukuran dunia virtual, misalnya `LOGICAL_WIDTH = 960` dan `LOGICAL_HEIGHT = 720`.
2. Hitung `scale = min(screen_width / LOGICAL_WIDTH, screen_height / LOGICAL_HEIGHT)`.
3. Hitung ukuran viewport: `viewport_width = LOGICAL_WIDTH * scale`, `viewport_height = LOGICAL_HEIGHT * scale`.
4. Posisikan viewport di tengah layar.
5. Render game ke logical canvas atau logical camera, lalu present/copy ke viewport tersebut.
6. Jangan memaksa lebar dan tinggi layar secara independen karena itu akan membuat game gepeng/terdistorsi.

## Library/framework relevan per bahasa

| Bahasa | Framework yang cocok | Catatan integrasi |
| --- | --- | --- |
| C++ | SDL2/SDL3, SFML, raylib, Unreal Engine | Cocok untuk port native dan berbagi aset dengan build C. |
| C# | MonoGame, FNA, Unity, Godot C# | Cocok untuk tooling editor, gameplay scripting, dan prototipe cepat. |
| Java | libGDX, jMonkeyEngine | Cocok untuk desktop/Android dengan pipeline aset matang. |
| Python | pygame-ce, arcade, Panda3D | Cocok untuk prototyping AI, balancing, dan tools pipeline. |
| JavaScript/TypeScript | Phaser, PixiJS, Three.js | Cocok untuk web build, UI, dan deployment browser. |
| Rust | Bevy, macroquad, ggez, SDL2 bindings | Cocok untuk safety, ECS modern, dan native/web via WASM. |
| Go | Ebiten, raylib-go | Cocok untuk binary kecil dan loop game sederhana. |
| Lua | LÖVE, Defold, Solar2D | Cocok untuk scripting ringan dan modding. |

## Integrasi antar bahasa

- **Shared data:** simpan konfigurasi level, wave, skor, dan balancing di JSON/TOML agar semua bahasa dapat membaca data yang sama.
- **Native bridge:** untuk memakai core C dari C++, C#, Python, Rust, Go, atau Java, expose API C kecil seperti `astro_init`, `astro_update`, dan `astro_snapshot`, lalu gunakan FFI (`ctypes`, P/Invoke, JNI, cgo, Rust FFI).
- **Scripting:** gunakan Lua atau JavaScript untuk rules/mission scripting, tetapi biarkan renderer memakai pola logical canvas yang sama.
- **Assets:** gunakan folder aset bersama (`assets/`) dan hindari path absolut agar setiap runtime mudah menjalankan contoh.

Lihat contoh minimal di `examples/multilanguage/*/aspect_fit.*` untuk implementasi per bahasa.
