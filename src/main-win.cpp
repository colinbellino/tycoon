#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

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

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

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
    GameMemory memory = {};
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

    {
        int permanentStorageSize = megabytes(2);
        memory.permanentStorage = malloc(permanentStorageSize);
        memory.permanentStorageSize = permanentStorageSize;
        memory.isInitialized = true;

        GameState *gameState = (GameState *)memory.permanentStorage;

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        gameState->window = SDL_CreateWindow(
            "Tycoon",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);
        if (gameState->window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        // SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_PING, "0");

        gameState->renderer = SDL_CreateRenderer(gameState->window, -1, SDL_RENDERER_ACCELERATED);
        gameState->screenSurface = SDL_GetWindowSurface(gameState->window);
        gameState->texture = SDL_CreateTexture(gameState->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1024, 768);

        SDL_SetWindowOpacity(gameState->window, 0.5f);
    }

    quit = game.start(&memory);

    while (quit == false)
    {
        quit = game.update(&memory);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                return 1;
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SHOWN:
                    SDL_Log("Window %d shown", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    SDL_Log("Window %d hidden", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_Log("Window %d exposed", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    SDL_Log("Window %d moved to %d,%d",
                            event.window.windowID, event.window.data1,
                            event.window.data2);
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    SDL_Log("Window %d resized to %dx%d",
                            event.window.windowID, event.window.data1,
                            event.window.data2);
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    SDL_Log("Window %d size changed to %dx%d",
                            event.window.windowID, event.window.data1,
                            event.window.data2);
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    SDL_Log("Window %d minimized", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    SDL_Log("Window %d maximized", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    SDL_Log("Window %d restored", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    SDL_Log("Mouse entered window %d",
                            event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    SDL_Log("Mouse left window %d", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    SDL_Log("Window %d gained keyboard focus",
                            event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    SDL_Log("Window %d lost keyboard focus",
                            event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    SDL_Log("Window %d closed", event.window.windowID);
                    break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
                case SDL_WINDOWEVENT_TAKE_FOCUS:
                    SDL_Log("Window %d is offered a focus", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_HIT_TEST:
                    SDL_Log("Window %d has a special hit test", event.window.windowID);
                    break;
#endif
                default:
                    SDL_Log("Window %d got unknown event %d",
                            event.window.windowID, event.window.event);
                    break;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    printf("space\n");
                    break;
                }
            }
            else
            {
                // printf("unkndown event: %i\n", event.key);
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