#include <stdio.h>
#include <SDL2/SDL.h>

#include "game.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;

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

    SDL_ShowWindow(window);

    return 0;
}

extern "C" GAME_UPDATE(gameUpdate)
{
    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                return 1;
            }
        }
    }

    char str[] = "game.update()";
    printf("%s\n", str);

    return 0;
}