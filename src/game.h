#if !defined(GAME_H)

#include <stdint.h>

#include "game_memory.h"
#include "game_utils.h"
#include "game_tilemap.h"

struct SDLStuff
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *screenSurface;
    SDL_Texture *rectTexture;
    SDL_Texture *spriteTexture;

    SDL_Texture *texture1;
    SDL_Texture *texture2;
};

struct GameMemory
{
    bool isInitialized;

    uint64_t permanentStorageSize;
    void *permanentStorage; // REQUIRED to be cleared to zero at startup.

    uint64_t transientStorageSize;
    void *transientStorage; // REQUIRED to be cleared to zero at startup.

    //  DebugPlatformReadFromFile *readFromFile;
    //  DebugPlatformWriteToFile *writeToFile;
};

struct World
{
    bool isOn;
    TileMap *tileMap;
};

struct GameState
{
    MemoryArena worldArena;
    World *world;

    uint32_t playerX;
    uint32_t playerY;

    int windowWidth;
    int windowHeight;
};

struct GameInput
{
    bool spaceWasPressedThisFrame;
};

#define GAME_UPDATE(name) int name(GameMemory *memory, GameInput input, SDLStuff *sdl)
typedef GAME_UPDATE(GameUpdate);

#define GAME_H
#endif
