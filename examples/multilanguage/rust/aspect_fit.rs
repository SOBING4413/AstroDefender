// Rust: framework-neutral aspect-fit helper for Bevy/macroquad/ggez/SDL2 bindings.
pub const LOGICAL_WIDTH: f32 = 960.0;
pub const LOGICAL_HEIGHT: f32 = 720.0;

#[derive(Debug, Clone, Copy)]
pub struct Viewport { pub x: f32, pub y: f32, pub width: f32, pub height: f32 }

pub fn aspect_fit_viewport(screen_width: f32, screen_height: f32) -> Viewport {
    let scale = (screen_width / LOGICAL_WIDTH).min(screen_height / LOGICAL_HEIGHT);
    let width = LOGICAL_WIDTH * scale;
    let height = LOGICAL_HEIGHT * scale;
    Viewport { x: (screen_width - width) * 0.5, y: (screen_height - height) * 0.5, width, height }
}
