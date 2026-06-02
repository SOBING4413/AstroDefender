# Polyglot Game Development Guide

AstroDefender tidak diposisikan sebagai proyek C-only. Build utama tetap C/SDL2, tetapi pengembangan multi-bahasa dilakukan melalui **satu kontrak polyglot bersama**, bukan potongan contoh yang berdiri sendiri.

Kontrak resmi ada di `polyglot/contract/astro_polyglot_manifest.json`. Semua modul C, C++, C#, Java, Python, JavaScript/TypeScript, Rust, Go, dan Lua wajib membaca atau mereplikasi nilai kontrak yang sama: logical canvas 960x720, aspect policy `fit-letterbox`, dan tick rate 60 Hz.

## Struktur polyglot wajib

```text
AstroDefender/
├─ include/config.h                         # C runtime contract constants
├─ src/                                     # SDL2 host executable
├─ polyglot/
│  ├─ contract/astro_polyglot_manifest.json # sumber kebenaran lintas bahasa
│  ├─ cpp-renderer/                         # C++ renderer adapter
│  ├─ csharp-gameplay/                      # C# gameplay DTO/bridge
│  ├─ java-assets/                          # Java asset validation pipeline
│  ├─ python-orchestrator/                  # Python contract orchestration
│  ├─ typescript-ui/                        # TS web/debug HUD bridge
│  ├─ rust-physics/                         # Rust FFI/WASM-ready physics helpers
│  ├─ go-telemetry/                         # Go telemetry payload service
│  └─ lua-missions/                         # Lua mission rules
└─ tools/verify_polyglot_contract.py        # CI-friendly contract check
```

## Alur kerja yang saling berkesinambungan

1. **C runtime** menjalankan game, menyimpan ukuran logical 960x720, dan mempresentasikan fullscreen secara proporsional.
2. **C++ renderer adapter** memakai kontrak yang sama untuk native renderer/engine port agar hasil viewport identik dengan C runtime.
3. **Rust physics module** menyediakan helper collision/aspect yang dapat diekspor via FFI/WASM saat runtime atau tooling membutuhkan determinisme tambahan.
4. **Lua mission scripting** mengubah rules wave dari snapshot state tanpa mengubah host runtime.
5. **C# gameplay bridge** membawa snapshot/command ke editor tooling MonoGame/FNA/Unity/Godot.
6. **Java asset pipeline** memvalidasi metadata aset agar sesuai versi kontrak sebelum dikemas.
7. **TypeScript HUD bridge** membaca snapshot runtime untuk overlay/debug web.
8. **Go telemetry service** mengemas skor, level, dan session data untuk leaderboard/analytics.
9. **Python orchestrator** memastikan semua modul masih berada di versi kontrak yang sama dan menjadi entry point automasi CI.

## Pola fullscreen yang dipakai semua modul

1. Simpan ukuran dunia virtual, `LOGICAL_WIDTH = 960` dan `LOGICAL_HEIGHT = 720`.
2. Hitung `scale = min(screen_width / LOGICAL_WIDTH, screen_height / LOGICAL_HEIGHT)`.
3. Hitung viewport tengah: `viewport_width = LOGICAL_WIDTH * scale`, `viewport_height = LOGICAL_HEIGHT * scale`.
4. Render game ke logical canvas/framebuffer/camera, lalu present ke viewport tersebut.
5. Area sisa menjadi letterbox/pillarbox. Jangan melakukan stretch X/Y terpisah.

## Verifikasi CI

Jalankan:

```bash
python3 tools/verify_polyglot_contract.py
```

Check ini gagal jika salah satu modul bahasa tidak membawa marker `ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0` sesuai manifest.
