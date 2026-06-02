// C# + MonoGame: use this destination rectangle when drawing a 960x720 RenderTarget2D.
using Microsoft.Xna.Framework;

public static class AspectFit {
    public const int LogicalWidth = 960;
    public const int LogicalHeight = 720;

    public static Rectangle Viewport(int backBufferWidth, int backBufferHeight) {
        float scale = System.MathF.Min(backBufferWidth / (float)LogicalWidth,
                                       backBufferHeight / (float)LogicalHeight);
        int width = (int)(LogicalWidth * scale + 0.5f);
        int height = (int)(LogicalHeight * scale + 0.5f);
        return new Rectangle((backBufferWidth - width) / 2, (backBufferHeight - height) / 2, width, height);
    }
}
