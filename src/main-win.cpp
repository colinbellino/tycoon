#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#if HOT_RELOAD
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#else
#include "game.cpp"
#endif

typedef struct
{
    HMODULE handle;
    GameStart *start;
    GameUpdate *update;
    bool isValid;
} WinGameCode;

#define print(...)       \
    printf(__VA_ARGS__); \
    fflush(stdout);

#if HOT_RELOAD
time_t now = time(0);
time_t lastModified = now;
time_t lastReload = now;

const char *gamePath = "./build/game.dll";
const char *gameCopyPath = "./build/game_tmp.dll";
const char *sourcePath = "./src/";

time_t fileTimeToTime(FILETIME const &fileTime)
{
    ULARGE_INTEGER value;
    value.LowPart = fileTime.dwLowDateTime;
    value.HighPart = fileTime.dwHighDateTime;
    return value.QuadPart / 10000000ULL - 11644473600ULL;
}

bool gameChanged()
{
    now = time(0);
    struct dirent *file;
    struct stat info;

    DIR *directory = opendir(sourcePath);
    if (directory == 0)
    {
        perror("The directory couldn't be accessed or does not exist.\n");
        return false;
    }

    while ((file = readdir(directory)))
    {
        char buffer[1024];
        strcpy(buffer, sourcePath);
        strcat(buffer, file->d_name);
        if (stat(buffer, &info) == -1)
        {
            perror(buffer);
            continue;
        }

        // print("%30s\n", ctime(&info.st_mtime));
        if (info.st_mtime > lastReload)
        {
            // print("%20s\n", file->d_name);
            return true;
        }
    }

    return false;
}

time_t getModificationTime(const char *filePath)
{
    FILETIME modificationTime = {};

    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFileA(filePath, &data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        modificationTime = data.ftLastWriteTime;
        FindClose(handle);
    }

    return fileTimeToTime(modificationTime);
}

WinGameCode loadGameCode()
{
    WinGameCode result = {};

    // Copy the dll just in case it's loaded by something else
    CopyFile(gamePath, gameCopyPath, FALSE);

    result.handle = LoadLibraryA(gameCopyPath);
    if (result.handle == 0)
    {
        fprintf(stderr, "Error while loading game library.\n");
        return result;
    }

    result.start = (GameStart *)GetProcAddress(result.handle, "gameStart");
    result.update = (GameUpdate *)GetProcAddress(result.handle, "gameUpdate");
    result.isValid = (result.start != NULL && result.update != NULL);

    if (result.isValid == 0)
    {
        result.start = gameStartStub;
        result.update = gameUpdateStub;
    }

    return result;
}

void unloadGameCode(WinGameCode *gameCode)
{
    if (gameCode->handle)
    {
        FreeLibrary(gameCode->handle);
        gameCode->handle = NULL;
    }

    gameCode->start = gameStartStub;
    gameCode->update = gameUpdateStub;
    gameCode->isValid = false;
}
#endif

int main(int argv, char *args[])
{
    WinGameCode game = {};
    bool quit = false;

#if HOT_RELOAD
    print("Starting game (with hot-reload).\n");
    game = loadGameCode();
#else
    game.start = gameStart;
    game.update = gameUpdate;
    game.isValid = true;
    print("Starting game.\n");
#endif

    GameMemory memory = {};
    int permanentStorageSize = megabytes(2);
    memory.permanentStorage = malloc(permanentStorageSize);
    memory.permanentStorageSize = permanentStorageSize;
    // memory.readFromFile = macDebugPlatformReadEntireFile;
    // memory.writeToFile = macDebugPlatformWriteEntireFile;
    memory.isInitialized = true;

    quit = game.start(&memory);

    while (quit == false)
    {
        quit = game.update(&memory);

#if HOT_RELOAD

        if (gameChanged())
        {
            printTime("start:", time(0));
            system("build-game-win.bat");

            print("Reloading game code.\n");
            unloadGameCode(&game);
            game = loadGameCode();
            printTime("end:", time(0));

            if (game.isValid == false)
            {
                quit = true;
            }

            lastReload = now;
        }
#endif
    }

    print("Exiting game.\n");
    print("============================================\n");

    return 0;
}