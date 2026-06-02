// AstroDefender polyglot module: TypeScript ui_overlay
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Consumes frame snapshots emitted by the runtime/orchestrator for browser debug HUDs.
export const CONTRACT_VERSION = "1.0.0";
export const LOGICAL_WIDTH = 960;
export const LOGICAL_HEIGHT = 720;

export type AstroFrameSnapshot = {
  tick: number;
  score: number;
  level: number;
  player: { x: number; y: number };
  contractVersion: typeof CONTRACT_VERSION;
};

export function aspectFitViewport(screenWidth: number, screenHeight: number) {
  const scale = Math.min(screenWidth / LOGICAL_WIDTH, screenHeight / LOGICAL_HEIGHT);
  const width = Math.round(LOGICAL_WIDTH * scale);
  const height = Math.round(LOGICAL_HEIGHT * scale);
  return { x: Math.floor((screenWidth - width) / 2), y: Math.floor((screenHeight - height) / 2), width, height };
}

export function renderHud(ctx: CanvasRenderingContext2D, snapshot: AstroFrameSnapshot) {
  const dst = aspectFitViewport(ctx.canvas.width, ctx.canvas.height);
  ctx.save();
  ctx.fillStyle = "rgb(0, 4, 18)";
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
  ctx.fillStyle = "rgb(120, 220, 255)";
  ctx.fillText(`Score ${snapshot.score} | Level ${snapshot.level}`, dst.x + 16, dst.y + 24);
  ctx.restore();
}
