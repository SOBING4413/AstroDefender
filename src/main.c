/*
 * AstroDefender - A Classic Space Shooter
 * Entry Point: main.c
 *
 * Initializes SDL2 subsystems, creates the window and renderer,
 * then delegates to the game loop.
 */

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "config.h"

int main(int argc, char* argv[])
{
    /* Suppress unused parameter warnings */
    (void)argc;
    (void)argv;

    /* Initialize SDL core */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    /* Initialize SDL_ttf for font rendering */
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Initialize SDL_mixer for audio */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        /* Non-fatal: game runs without audio */
    }

    /* Create window */
    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Create hardware-accelerated renderer with vsync */
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Set logical render size for consistent scaling */
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    /* Run the game */
    Game_Run(renderer);

    /* Cleanup */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
