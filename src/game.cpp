#include <stdio.h>
#include <SDL2/SDL.h>

#include "game.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

extern "C" GAME_START(gameStart)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "Tycoon",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    screenSurface = SDL_GetWindowSurface(window);

    SDL_ShowWindow(window);

    return 0;
}

extern "C" GAME_UPDATE(gameUpdate)
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            return 1;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_RenderClear(renderer); // fill the scene with white

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // the rect color (solid red)
    SDL_Rect rect = {0, 0, 50, 50};                     // the rectangle
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderPresent(renderer); // copy to screen

    // char str[] = "game.update()";
    // printf("%s\n", str);

    return 0;
}