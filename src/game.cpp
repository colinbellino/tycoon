#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "game.h"

#define assert(expression, message)                                                    \
    if (expression == 0)                                                               \
    {                                                                                  \
        printf("Assertion failed in %s, line %d: %s.\n", __FILE__, __LINE__, message); \
    }

extern "C" int gameStart(GameMemory *memory)
{
    return 0;
}

extern "C" int gameUpdate(GameMemory *memory)
{
    assert((sizeof(GameState) <= memory->permanentStorageSize), "GameState can't be larger than permanent storage");

    GameState *gameState = (GameState *)memory->permanentStorage;

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

    SDL_SetRenderDrawColor(gameState->renderer, 0xFF, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(gameState->renderer, &rect);
    SDL_SetRenderTarget(gameState->renderer, NULL);
    SDL_RenderCopy(gameState->renderer, gameState->texture, NULL, NULL);

    SDL_RenderPresent(gameState->renderer);

    // char str[] = "game.update()";
    // printf("%s\n", str);

    // SDL_Log("update\n");
    // SDL_Delay(1000.0f);

    return 0;
}