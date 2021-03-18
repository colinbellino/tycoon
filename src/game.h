#if !defined(GAME_H)

#include <stdint.h>

typedef struct
{
    bool isInitialized;

    uint64_t permanentStorageSize;
    void *permanentStorage;

    uint64_t transientStorageSize;
    void *transientStorage;

    //  DebugPlatformReadFromFile *readFromFile;
    //  DebugPlatformWriteToFile *writeToFile;
} GameMemory;

// TODO: Don't use SDL directly inside the game code
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *screenSurface;
    SDL_Texture *rectTexture;
    SDL_Texture *spriteTexture;

    int debug;
} GameState;

#define GAME_START(name) int name(GameMemory *memory)
typedef GAME_START(GameStart);

#define GAME_UPDATE(name) int name(GameMemory *memory)
typedef GAME_UPDATE(GameUpdate);

#define GAME_H
#endif
