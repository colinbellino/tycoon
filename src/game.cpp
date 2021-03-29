#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

    {
        SDL_RenderClear(gameState->renderer);
        SDL_SetRenderTarget(gameState->renderer, gameState->spriteTexture);
        SDL_RenderCopy(gameState->renderer, gameState->spriteTexture, NULL, NULL);
        SDL_RenderPresent(gameState->renderer);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            return 1;
        }
        if (event.type == SDL_WINDOWEVENT)
        {
            //             switch (event.window.event)
            //             {
            //             case SDL_WINDOWEVENT_SHOWN:
            //                SDL_Log("Window %d shown", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_HIDDEN:
            //                SDL_Log("Window %d hidden", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_EXPOSED:
            //                SDL_Log("Window %d exposed", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_MOVED:
            //                SDL_Log("Window %d moved to %d,%d",
            //                        event.window.windowID, event.window.data1,
            //                        event.window.data2);
            //                break;
            //             case SDL_WINDOWEVENT_RESIZED:
            //                SDL_Log("Window %d resized to %dx%d",
            //                        event.window.windowID, event.window.data1,
            //                        event.window.data2);
            //                break;
            //             case SDL_WINDOWEVENT_SIZE_CHANGED:
            //                SDL_Log("Window %d size changed to %dx%d",
            //                        event.window.windowID, event.window.data1,
            //                        event.window.data2);
            //                break;
            //             case SDL_WINDOWEVENT_MINIMIZED:
            //                SDL_Log("Window %d minimized", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_MAXIMIZED:
            //                SDL_Log("Window %d maximized", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_RESTORED:
            //                SDL_Log("Window %d restored", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_ENTER:
            //                SDL_Log("Mouse entered window %d",
            //                        event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_LEAVE:
            //                SDL_Log("Mouse left window %d", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_FOCUS_GAINED:
            //                SDL_Log("Window %d gained keyboard focus",
            //                        event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_FOCUS_LOST:
            //                SDL_Log("Window %d lost keyboard focus",
            //                        event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_CLOSE:
            //                SDL_Log("Window %d closed", event.window.windowID);
            //                break;
            // #if SDL_VERSION_ATLEAST(2, 0, 5)
            //             case SDL_WINDOWEVENT_TAKE_FOCUS:
            //                SDL_Log("Window %d is offered a focus", event.window.windowID);
            //                break;
            //             case SDL_WINDOWEVENT_HIT_TEST:
            //                SDL_Log("Window %d has a special hit test", event.window.windowID);
            //                break;
            // #endif
            //             default:
            //                SDL_Log("Window %d got unknown event %d",
            //                        event.window.windowID, event.window.event);
            //                break;
            //             }
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_SPACE:
                print("space pressed!\n");
                break;
            }
        }
        else
        {
            // printf("unkndown event: %i\n", event.key);
        }
    }
    // {
    //     SDL_Rect rect;
    //     rect.x = rand() % 500;
    //     rect.y = rand() % 500;
    //     rect.w = 100;
    //     rect.h = 50;

    //     SDL_SetRenderTarget(gameState->renderer, gameState->rectTexture);

    //     SDL_SetRenderDrawColor(gameState->renderer, 0x00, 0x00, 0x00, 0x00);
    //     SDL_RenderClear(gameState->renderer);
    //     SDL_RenderDrawRect(gameState->renderer, &rect);

    //     SDL_SetRenderDrawColor(gameState->renderer, 0xFF, 0x00, 0x00, 0x00);
    //     SDL_RenderFillRect(gameState->renderer, &rect);
    //     SDL_SetRenderTarget(gameState->renderer, NULL);

    //     SDL_RenderCopy(gameState->renderer, gameState->rectTexture, NULL, NULL);
    //     SDL_RenderPresent(gameState->renderer);
    // }

    return 0;
}