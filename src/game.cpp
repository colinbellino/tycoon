#include <stdio.h>

#if __APPLE__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include "game.h"

extern "C" int gameUpdate(GameMemory *memory, GameInput input, SDLStuff *sdl)
{
    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        print("Initializing game.\n");
        srand(time(NULL));

        initializeArena(
            &gameState->worldArena,
            (uint8_t *)memory->permanentStorage + sizeof(GameState),
            memory->permanentStorageSize - sizeof(GameState));
        gameState->world = pushStruct(&gameState->worldArena, World);

        gameState->playerX = 15;
        gameState->playerY = 15;

        World *world = gameState->world;
        world->tileMap = pushStruct(&gameState->worldArena, TileMap);

        TileMap *tileMap = world->tileMap;
        tileMap->chunkShift = 4;
        tileMap->chunkMask = (1 << tileMap->chunkShift) - 1;
        tileMap->chunkDimension = (1 << tileMap->chunkShift);

        tileMap->tileChunkCountX = 128;
        tileMap->tileChunkCountY = 128;
        tileMap->tileChunkCountZ = 2;
        tileMap->tileChunks = pushArray(
            &gameState->worldArena,
            tileMap->tileChunkCountX * tileMap->tileChunkCountY * tileMap->tileChunkCountZ,
            TileChunk);

        tileMap->tileSizeInMeters = 1.0;

        uint32_t TilesPerWidth = 17;
        uint32_t TilesPerHeight = 9;
        uint32_t ScreenX = 0;
        uint32_t ScreenY = 0;
        uint32_t AbsTileZ = 0;

        // TODO(casey): Replace all this with real world generation!
        bool DoorLeft = false;
        bool DoorRight = false;
        bool DoorTop = false;
        bool DoorBottom = false;
        bool DoorUp = false;
        bool DoorDown = false;

        for (uint32_t ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex)
        {
            uint32_t RandomChoice;
            if (DoorUp || DoorDown)
            {
                RandomChoice = rand() % 2;
            }
            else
            {
                RandomChoice = rand() % 3;
            }

            bool CreatedZDoor = false;
            if (RandomChoice == 2)
            {
                CreatedZDoor = true;
                if (AbsTileZ == 0)
                {
                    DoorUp = true;
                }
                else
                {
                    DoorDown = true;
                }
            }
            else if (RandomChoice == 1)
            {
                DoorRight = true;
            }
            else
            {
                DoorTop = true;
            }

            for (uint32_t TileY = 0; TileY < TilesPerHeight; ++TileY)
            {
                for (uint32_t TileX = 0; TileX < TilesPerWidth; ++TileX)
                {
                    uint32_t AbsTileX = ScreenX * TilesPerWidth + TileX;
                    uint32_t AbsTileY = ScreenY * TilesPerHeight + TileY;

                    Tile tile;
                    tile.value = 0;

                    if ((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight / 2))))
                    {
                        tile.value = 2;
                    }

                    if ((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight / 2))))
                    {
                        tile.value = 2;
                    }

                    if ((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth / 2))))
                    {
                        tile.value = 2;
                    }

                    if ((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth / 2))))
                    {
                        tile.value = 2;
                    }

                    if ((TileX == 10) && (TileY == 6))
                    {
                        if (DoorUp)
                        {
                            tile.value = 3;
                        }

                        if (DoorDown)
                        {
                            tile.value = 4;
                        }
                    }

                    setTileValue(&gameState->worldArena, world->tileMap, AbsTileX, AbsTileY, AbsTileZ, tile);
                }
            }

            DoorLeft = DoorRight;
            DoorBottom = DoorTop;

            if (CreatedZDoor)
            {
                DoorDown = !DoorDown;
                DoorUp = !DoorUp;
            }
            else
            {
                DoorUp = false;
                DoorDown = false;
            }

            DoorRight = false;
            DoorTop = false;

            if (RandomChoice == 2)
            {
                if (AbsTileZ == 0)
                {
                    AbsTileZ = 1;
                }
                else
                {
                    AbsTileZ = 0;
                }
            }
            else if (RandomChoice == 1)
            {
                ScreenX += 1;
            }
            else
            {
                ScreenY += 1;
            }
        }

        memory->isInitialized = true;
    }

    World *world = gameState->world;
    TileMap *tileMap = world->tileMap;

    world->isOn = input.spaceWasPressedThisFrame;

    int32_t tileSizeInPixels = 10;
    float metersToPixels = (float)tileSizeInPixels / (float)tileMap->tileSizeInMeters;
    float screenCenterX = gameState->windowWidth / 2;
    float screenCenterY = gameState->windowHeight / 2;

    SDL_SetRenderTarget(sdl->renderer, sdl->rectTexture);
    SDL_SetRenderDrawColor(sdl->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(sdl->renderer);
    // SDL_RenderDrawRect(sdl->renderer, &rect);

    for (int32_t RelRow = -30; RelRow < 30; ++RelRow)
    {
        for (int32_t RelColumn = -40; RelColumn < 40; ++RelColumn)
        {
            uint32_t Column = gameState->playerX + RelColumn;
            // uint32_t Column = 0 + RelColumn;
            uint32_t Row = gameState->playerY + RelRow;
            // uint32_t Row = 0 + RelRow;
            Tile tile = getTileValue(tileMap, Column, Row, 0);

            if (tile.value > 1)
            {
                uint8_t Gray = 0xAA;
                if (tile.value == 2)
                {
                    Gray = 0xFF;
                }

                if (tile.value > 2)
                {
                    Gray = 0x33;
                }

                if (
                    (Column == gameState->playerX) &&
                    (Row == gameState->playerY))
                {
                    Gray = 0x00;
                }

                float CenX = screenCenterX - metersToPixels * gameState->playerX + ((float)RelColumn) * tileSizeInPixels;
                float CenY = screenCenterY + metersToPixels * gameState->playerY - ((float)RelRow) * tileSizeInPixels;
                float MinX = CenX - 0.5f * tileSizeInPixels;
                float MinY = CenY - 0.5f * tileSizeInPixels;
                float MaxX = CenX + 0.5f * tileSizeInPixels;
                float MaxY = CenY + 0.5f * tileSizeInPixels;

                SDL_Rect rect;
                rect.w = tileSizeInPixels;
                rect.h = tileSizeInPixels;
                rect.x = CenX;
                rect.y = CenY;

                SDL_SetRenderTarget(sdl->renderer, sdl->rectTexture);

                // SDL_RenderClear(sdl->renderer);
                // SDL_SetRenderDrawColor(sdl->renderer, 0x00, 0x00, 0x00, 0x00);
                // SDL_RenderDrawRect(sdl->renderer, &rect);

                SDL_SetRenderDrawColor(sdl->renderer, Gray, Gray, Gray, 0x00);

                SDL_RenderFillRect(sdl->renderer, &rect);
                SDL_SetRenderTarget(sdl->renderer, NULL);

                SDL_RenderCopy(sdl->renderer, sdl->rectTexture, NULL, NULL);
            }
        }
    }

    SDL_RenderPresent(sdl->renderer);

    return 0;
}