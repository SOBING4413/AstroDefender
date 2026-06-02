// TypeScript + Canvas/Phaser-style rendering: draw logical canvas into a centered viewport.
export const LOGICAL_WIDTH = 960;
export const LOGICAL_HEIGHT = 720;

export function aspectFitViewport(screenWidth: number, screenHeight: number) {
  const scale = Math.min(screenWidth / LOGICAL_WIDTH, screenHeight / LOGICAL_HEIGHT);
  const width = Math.round(LOGICAL_WIDTH * scale);
  const height = Math.round(LOGICAL_HEIGHT * scale);
  return { x: Math.floor((screenWidth - width) / 2), y: Math.floor((screenHeight - height) / 2), width, height };
}

export function present(ctx: CanvasRenderingContext2D, logicalCanvas: HTMLCanvasElement) {
  const dst = aspectFitViewport(ctx.canvas.width, ctx.canvas.height);
  ctx.fillStyle = "rgb(0, 4, 18)";
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
  ctx.drawImage(logicalCanvas, dst.x, dst.y, dst.width, dst.height);
}
