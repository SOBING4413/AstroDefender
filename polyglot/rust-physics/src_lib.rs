// AstroDefender polyglot module: Rust physics_module
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Can be exported through FFI/WASM while sharing the same dimensions as the C runtime.
pub const CONTRACT_VERSION: &str = "1.0.0";
pub const LOGICAL_WIDTH: f32 = 960.0;
pub const LOGICAL_HEIGHT: f32 = 720.0;

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct Viewport { pub x: f32, pub y: f32, pub width: f32, pub height: f32 }

#[no_mangle]
pub extern "C" fn astro_aspect_fit_viewport(screen_width: f32, screen_height: f32) -> Viewport {
    let scale = (screen_width / LOGICAL_WIDTH).min(screen_height / LOGICAL_HEIGHT);
    let width = LOGICAL_WIDTH * scale;
    let height = LOGICAL_HEIGHT * scale;
    Viewport { x: (screen_width - width) * 0.5, y: (screen_height - height) * 0.5, width, height }
}

#[no_mangle]
pub extern "C" fn astro_aabb_overlap(ax: f32, ay: f32, aw: f32, ah: f32, bx: f32, by: f32, bw: f32, bh: f32) -> bool {
    ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by
}
