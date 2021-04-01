#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"

#if HOT_RELOAD
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#else
#include "game.cpp"
#endif

typedef struct
{
    void *handle;
    GameUpdate *update;
    bool isValid;
} MacGameCode;

int gameUpdateStub(GameMemory *memory, GameInput input, SDLStuff *sdl)
{
    printf("Using stub for game.update()\n");
    return 1;
}

#if HOT_RELOAD
time_t now = time(0);
time_t lastModified = now;
time_t lastReload = now;

const char *gamePath = "./build/game.o";
const char *sourcePath = "./src/";
const char *buildCommand = "./mac_build_game.sh";

time_t getFileCreationTime(const char *filePath)
{
    struct stat attrib;
    stat(filePath, &attrib);
    return attrib.st_ctime;
}

bool gameChanged()
{
    now = time(0);
    DIR *directory = opendir(sourcePath);
    struct dirent *entry;
    struct stat info;

    if (directory == NULL)
    {
        fprintf(stderr, "Could not open source directory.\n");
        return false;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        char buffer[1024];
        strcpy(buffer, sourcePath);
        strcat(buffer, entry->d_name);

        if (stat(buffer, &info) == -1)
        {
            // perror(buffer);
            continue;
        }

        // print("%30s\n", ctime(&info.st_mtime));
        if (info.st_mtime > lastReload)
        {
            // print("%20s\n", file->d_name);
            return true;
        }
    }

    closedir(directory);
    return false;
}

MacGameCode loadGameCode()
{
    MacGameCode result = {};

    result.handle = dlopen(gamePath, RTLD_LAZY);
    if (!result.handle)
    {
        fprintf(stderr, "Error: %s\n", dlerror());
        return result;
    }

    result.update = (GameUpdate *)dlsym(result.handle, "gameUpdate");
    result.isValid = (result.update != NULL);

    return result;
}

void unloadGameCode(MacGameCode *gameCode)
{
    if (gameCode->handle)
    {
        dlclose(gameCode->handle);
        gameCode->handle = NULL;
    }

    gameCode->update = gameUpdateStub;
    gameCode->isValid = false;
}
#endif

int main()
{
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 450;
    bool quit = false;
    MacGameCode game = {};
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
        quit = game.update(&memory, input, &sdl);

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
            system(buildCommand);
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

    printf("Exiting game.\n");
    printf("=================================\n");

    return 0;
}