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

#define GAME_START(name) int name()
typedef GAME_START(GameStart);

#define GAME_UPDATE(name) int name(GameMemory *memory)
typedef GAME_UPDATE(GameUpdate);

#define GAME_H
#endif
