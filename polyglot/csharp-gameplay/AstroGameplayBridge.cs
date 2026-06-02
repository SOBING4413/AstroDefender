// AstroDefender polyglot module: C# gameplay_bridge
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Consumes the same frame contract used by C, C++, TS, Rust, Go, Java, Python, and Lua.
using Microsoft.Xna.Framework;

namespace AstroDefender.Polyglot;

public readonly record struct AstroFrameSnapshot(
    int Tick,
    int Score,
    int Level,
    int PlayerX,
    int PlayerY,
    int LogicalWidth = AstroGameplayBridge.LogicalWidth,
    int LogicalHeight = AstroGameplayBridge.LogicalHeight
);

public static class AstroGameplayBridge {
    public const string ContractVersion = "1.0.0";
    public const int LogicalWidth = 960;
    public const int LogicalHeight = 720;

    public static Rectangle AspectFitViewport(int backBufferWidth, int backBufferHeight) {
        float scale = System.MathF.Min(backBufferWidth / (float)LogicalWidth,
                                       backBufferHeight / (float)LogicalHeight);
        int width = (int)(LogicalWidth * scale + 0.5f);
        int height = (int)(LogicalHeight * scale + 0.5f);
        return new Rectangle((backBufferWidth - width) / 2, (backBufferHeight - height) / 2, width, height);
    }
}
