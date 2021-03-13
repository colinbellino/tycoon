#include <stdio.h>
#include <SDL2/SDL.h>

#include "game.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *screenSurface;
    SDL_Texture *texture;

    int debug;
} GameState;

#define assert(expression, message)                                                    \
    if (expression == 0)                                                               \
    {                                                                                  \
        printf("Assertion failed in %s, line %d: %s.\n", __FILE__, __LINE__, message); \
    }

extern "C" int gameStart(GameMemory *memory)
{
    assert(sizeof(GameState) <= memory->permanentStorageSize, "GameState can't be larger than permanent storage");

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    gameState->window = SDL_CreateWindow(
        "Tycoon",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);
    if (gameState->window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    gameState->renderer = SDL_CreateRenderer(gameState->window, -1, 0);
    gameState->screenSurface = SDL_GetWindowSurface(gameState->window);
    gameState->texture = SDL_CreateTexture(gameState->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1024, 768);

    SDL_SetWindowOpacity(gameState->window, 0.5f);

    return 0;
}

extern "C" int gameUpdate(GameMemory *memory)
{
    assert(sizeof(GameState) <= memory->permanentStorageSize, "GameState can't be larger than permanent storage");

    GameState *gameState = (GameState *)memory->permanentStorage;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            return 1;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_SPACE:
                printf("space\n");
                break;
            }
        }
    }

    SDL_Rect rect;
    rect.x = rand() % 500;
    rect.y = rand() % 500;
    rect.w = 100;
    rect.h = 50;
    // printf("= %d %d\n", rect.x, rect.y);

    SDL_SetRenderTarget(gameState->renderer, gameState->texture);
    SDL_SetRenderDrawColor(gameState->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(gameState->renderer);
    SDL_RenderDrawRect(gameState->renderer, &rect);

    SDL_SetRenderDrawColor(gameState->renderer, 0x00, 0x00, 0xFF, 0x00);
    SDL_RenderFillRect(gameState->renderer, &rect);
    SDL_SetRenderTarget(gameState->renderer, NULL);
    SDL_RenderCopy(gameState->renderer, gameState->texture, NULL, NULL);

    SDL_RenderPresent(gameState->renderer);

    // char str[] = "game.update()";
    // printf("%s\n", str);

    return 0;
}