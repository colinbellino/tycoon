#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"

SDL_Texture *loadFromFile(const char *path, SDL_Renderer *renderer)
{
    SDL_Texture *texture = NULL;

    SDL_Surface *loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }

    SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

    texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (texture == NULL)
    {
        printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        return NULL;
    }

    // SDL_FreeSurface(loadedSurface);

    return texture;
}

bool loadMedia(GameState *gameState)
{
    printf("Loading all media...\n");
    bool success = true;

    {
        gameState->texture1 = loadFromFile("media/smile.png", gameState->renderer);
        if (gameState->texture1 == NULL)
        {
            printf("Failed to load \"media/smile.png\" texture image!\n");
            success = false;
        }
    }

    {
        gameState->texture2 = loadFromFile("media/smile.png", gameState->renderer);
        if (gameState->texture2 == NULL)
        {
            printf("Failed to load \"media/smile.png\" texture image!\n");
            success = false;
        }
    }

    return success;
}

extern "C" int gameStart(GameMemory *memory)
{
    // assert((sizeof(GameState) <= memory->permanentStorageSize), "GameState can't be larger than permanent storage");

    // GameState *gameState = (GameState *)memory->permanentStorage;

    // if (loadMedia(gameState) == false)
    // {
    //     return 1;
    // }

    return 0;
}

extern "C" int gameUpdate(GameMemory *memory, GameInput input)
{
    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        initializeArena(
            &gameState->worldArena,
            (uint8_t *)memory->permanentStorage + sizeof(GameState),
            memory->permanentStorageSize - sizeof(GameState));
        gameState->world = PUSH_STRUCT(&gameState->worldArena, World);

        memory->isInitialized = true;
    }

    gameState->world->isOn = input.spaceWasPressedThisFrame;

    {
        SDL_Rect gameplayViewport;
        gameplayViewport.x = 0;
        gameplayViewport.y = 0;
        gameplayViewport.w = gameState->windowWidth;
        gameplayViewport.h = gameState->windowHeight;
        SDL_RenderSetViewport(gameState->renderer, &gameplayViewport);
        SDL_RenderCopy(gameState->renderer, gameState->spriteTexture, NULL, NULL);

        SDL_Rect rect;
        rect.w = 300;
        rect.h = 200;
        rect.x = (gameState->windowWidth / 2 - rect.w / 2);
        rect.y = (gameState->windowHeight / 2 - rect.h / 2);
        // rect.x = rand() % (gameState->windowWidth - rect.w);
        // rect.y = rand() % (gameState->windowHeight - rect.h);

        SDL_SetRenderTarget(gameState->renderer, gameState->rectTexture);

        SDL_SetRenderDrawColor(gameState->renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(gameState->renderer);
        SDL_RenderDrawRect(gameState->renderer, &rect);

        if (gameState->world->isOn)
        {
            SDL_SetRenderDrawColor(gameState->renderer, 0x00, 0xFF, 0x00, 0x00);
        }
        else
        {
            SDL_SetRenderDrawColor(gameState->renderer, 0xFF, 0x00, 0x00, 0x00);
        }

        SDL_RenderFillRect(gameState->renderer, &rect);
        SDL_SetRenderTarget(gameState->renderer, NULL);

        SDL_RenderCopy(gameState->renderer, gameState->rectTexture, NULL, NULL);
    }

    // {
    //     // SDL_RenderClear(gameState->renderer);
    //     SDL_SetRenderTarget(gameState->renderer, gameState->spriteTexture);

    //     SDL_Rect uiViewport;
    //     uiViewport.x = 0;
    //     uiViewport.y = 0;
    //     uiViewport.w = gameState->windowWidth;
    //     uiViewport.h = 100;
    //     SDL_RenderSetViewport(gameState->renderer, &uiViewport);
    //     SDL_RenderCopy(gameState->renderer, gameState->spriteTexture, NULL, NULL);

    //     // SDL_RenderPresent(gameState->renderer);
    // }

    SDL_RenderPresent(gameState->renderer);

    return 0;
}