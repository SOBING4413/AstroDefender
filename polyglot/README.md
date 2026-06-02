# AstroDefender Polyglot Runtime

Folder ini **bukan kumpulan contoh terpisah**. Setiap bahasa adalah modul yang wajib mengikuti `polyglot/contract/astro_polyglot_manifest.json` agar bisa bekerja dalam satu pipeline AstroDefender.

## Kontrak bersama

Semua modul berbagi nilai berikut:

- `contract_version`: `1.0.0`
- logical gameplay canvas: `960x720`
- aspect policy: `fit-letterbox`
- tick rate: `60 Hz`

Python orchestrator memverifikasi bahwa modul C, C++, C#, Java, Python, TypeScript, Rust, Go, dan Lua masih memakai versi kontrak yang sama sebelum pipeline polyglot dijalankan.

```bash
python3 polyglot/python-orchestrator/astro_polyglot_orchestrator.py
```

## Peran wajib tiap bahasa

| Bahasa | Modul | Peran dalam satu pipeline |
| --- | --- | --- |
| C | `include/config.h`, `src/` | Host runtime SDL2, main loop, save data, dan fullscreen presentation. |
| C++ | `cpp-renderer/` | Adapter renderer native/engine yang memakai viewport math sama dengan runtime. |
| C# | `csharp-gameplay/` | DTO gameplay dan jembatan untuk MonoGame/FNA/Unity/Godot tooling. |
| Java | `java-assets/` | Validasi metadata aset sebelum dipakai runtime dan tools. |
| Python | `python-orchestrator/` | Orkestrasi kontrak dan pertukaran data antar modul. |
| JavaScript/TypeScript | `typescript-ui/` | Overlay HUD/debug web yang membaca snapshot runtime. |
| Rust | `rust-physics/` | Helper physics/collision deterministik yang siap FFI/WASM. |
| Go | `go-telemetry/` | Packaging telemetry skor/sesi untuk leaderboard atau analytics. |
| Lua | `lua-missions/` | Rules misi dan modifier wave yang dapat dibaca host runtime. |

Jika sebuah modul mengubah ukuran logical canvas, versi kontrak, atau format snapshot, semua modul lain harus diperbarui bersama-sama.
