#if !defined(GAME_H)

#include <stdint.h>

#include "memory.h"
#include "utils.h"

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
};

// TODO: Don't use SDL directly inside the game code
struct GameState
{
    MemoryArena worldArena;
    World *world;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *screenSurface;
    SDL_Texture *rectTexture;
    SDL_Texture *spriteTexture;

    SDL_Texture *texture1;
    SDL_Texture *texture2;

    int windowWidth;
    int windowHeight;
};

struct GameInput
{
    bool spaceWasPressedThisFrame;
};

#define GAME_START(name) int name(GameMemory *memory)
typedef GAME_START(GameStart);

#define GAME_UPDATE(name) int name(GameMemory *memory, GameInput input)
typedef GAME_UPDATE(GameUpdate);

#define GAME_H
#endif
