#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "game.h"

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
    GameUpdate *update;
    bool isValid;
} WinGameCode;

int gameUpdateStub(GameMemory *memory, GameInput input, SDLStuff *sdl)
{
    printf("Using stub for game.update()\n");
    return 1;
}

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
    const int WINDOW_WIDTH = 960;
    const int WINDOW_HEIGHT = 540;
    uint32_t ticksBefore = SDL_GetTicks();
    uint32_t ticksAfter = SDL_GetTicks();
    double delta = 0;
    bool quit = false;
    WinGameCode game = {};
    GameMemory memory = {};
    SDLStuff sdl = {};
    GameInput input = {};

#if HOT_RELOAD
    print("Starting game (with hot-reload).\n");
    game = loadGameCode();
#else
    game.update = gameUpdate;
    game.isValid = true;
    print("Starting game.\n");
#endif

    int permanentStorageSize = megabytes(2);
    memory.permanentStorage = malloc(permanentStorageSize);
    memory.permanentStorageSize = permanentStorageSize;

    GameState *gameState = (GameState *)memory.permanentStorage;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    gameState->windowWidth = WINDOW_WIDTH;
    gameState->windowHeight = WINDOW_HEIGHT;

    sdl.window = SDL_CreateWindow(
        "Tycoon",
        displayMode.w - WINDOW_WIDTH, 0,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);
    if (sdl.window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);
    sdl.screenSurface = SDL_GetWindowSurface(sdl.window);
    sdl.rectTexture = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_SetWindowOpacity(sdl.window, 0.5f);

    while (quit == false)
    {
        ticksBefore = SDL_GetTicks();
        delta += ticksBefore - ticksAfter;

        if (delta >= 1000 / 60.0)
        {
            print("FPS: %f\n", 1000 / delta);

            quit = game.update(&memory, input, &sdl);
            delta = 0;
        }

        ticksAfter = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    gameState->windowWidth = event.window.data1;
                    gameState->windowHeight = event.window.data2;
                    SDL_Log("Window %d resized to %dx%d",
                            event.window.windowID, event.window.data1,
                            event.window.data2);
                    break;
                }
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    gameState->windowWidth = event.window.data1;
                    gameState->windowHeight = event.window.data2;
                    SDL_Log("Window %d size changed to %dx%d",
                            event.window.windowID, event.window.data1,
                            event.window.data2);
                    break;
                }
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    input.spaceWasPressedThisFrame = false;
                    break;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    input.spaceWasPressedThisFrame = true;
                    break;
                case SDLK_RETURN:
                    memory.isInitialized = false;
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                }
            }
        }

#if HOT_RELOAD
        if (gameChanged())
        {
            system("build-game-win.bat");
            // For some reason, we need a short sleep after the task is done.
            usleep(1 * 1000);

            print("Reloading game code.\n");
            unloadGameCode(&game);
            game = loadGameCode();

            quit = !game.isValid;
            lastReload = now;
        }
#endif
    }

    print("Exiting game.\n");
    print("============================================\n");

    return 0;
}