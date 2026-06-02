// C++ + SDL2: render logical framebuffer to an aspect-preserving fullscreen viewport.
#include <SDL.h>
#include <algorithm>

constexpr int LOGICAL_WIDTH = 960;
constexpr int LOGICAL_HEIGHT = 720;

SDL_Rect aspectFitViewport(int outputW, int outputH) {
    const float scale = std::min(outputW / float(LOGICAL_WIDTH), outputH / float(LOGICAL_HEIGHT));
    const int w = int(LOGICAL_WIDTH * scale + 0.5f);
    const int h = int(LOGICAL_HEIGHT * scale + 0.5f);
    return SDL_Rect{(outputW - w) / 2, (outputH - h) / 2, w, h};
}

void present(SDL_Renderer* renderer, SDL_Texture* logicalFrame) {
    int outputW = 0, outputH = 0;
    SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
    SDL_Rect dst = aspectFitViewport(outputW, outputH);
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 4, 18, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, logicalFrame, nullptr, &dst);
    SDL_RenderPresent(renderer);
}
