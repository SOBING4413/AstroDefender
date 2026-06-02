// AstroDefender polyglot module: C++ renderer_adapter
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Cooperates with the C runtime by using the same 960x720 logical frame contract.
#include <SDL.h>
#include <algorithm>

namespace astro {
constexpr int kLogicalWidth = 960;
constexpr int kLogicalHeight = 720;

struct FrameContract {
    int logicalWidth = kLogicalWidth;
    int logicalHeight = kLogicalHeight;
    const char* aspectPolicy = "fit-letterbox";
};

SDL_Rect AspectFitViewport(int outputW, int outputH) {
    const float scale = std::min(outputW / float(kLogicalWidth), outputH / float(kLogicalHeight));
    const int w = int(kLogicalWidth * scale + 0.5f);
    const int h = int(kLogicalHeight * scale + 0.5f);
    return SDL_Rect{(outputW - w) / 2, (outputH - h) / 2, w, h};
}

void PresentLogicalFrame(SDL_Renderer* renderer, SDL_Texture* logicalFrame) {
    int outputW = 0;
    int outputH = 0;
    SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
    SDL_Rect dst = AspectFitViewport(outputW, outputH);
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 4, 18, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, logicalFrame, nullptr, &dst);
    SDL_RenderPresent(renderer);
}
}  // namespace astro
