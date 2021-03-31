#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "main.h"

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
   GameStart *start;
   GameUpdate *update;
   bool isValid;
} MacGameCode;

SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer)
{
   SDL_Texture *newTexture = NULL;

   SDL_Surface *loadedSurface = IMG_Load(path);
   if (loadedSurface == NULL)
   {
      printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
   }
   else
   {
      newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
      if (newTexture == NULL)
      {
         printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
      }

      SDL_FreeSurface(loadedSurface);
   }

   return newTexture;
}

#if HOT_RELOAD
time_t now = time(0);
time_t lastModified = now;
time_t lastReload = now;

const char *gamePath = "./build/game.o";
const char *sourcePath = "./src/";
const char *buildCommand = "./build-game-mac.sh";

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

   result.start = (GameStart *)dlsym(result.handle, "gameStart");
   result.update = (GameUpdate *)dlsym(result.handle, "gameUpdate");
   result.isValid = (result.start != NULL && result.update != NULL);

   return result;
}

void unloadGameCode(MacGameCode *gameCode)
{
   if (gameCode->handle)
   {
      dlclose(gameCode->handle);
      gameCode->handle = NULL;
   }

   gameCode->start = gameStartStub;
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
   GameInput input;

#if HOT_RELOAD
   print("Starting game (with hot-reload).\n");
   game = loadGameCode();
#else
   game.start = gameStart;
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

   gameState->window = SDL_CreateWindow(
       "Tycoon",
       displayMode.w - WINDOW_WIDTH, 0,
       WINDOW_WIDTH, WINDOW_HEIGHT,
       SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);
   if (gameState->window == NULL)
   {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      return 1;
   }

   gameState->windowWidth = WINDOW_WIDTH;
   gameState->windowHeight = WINDOW_HEIGHT;
   gameState->renderer = SDL_CreateRenderer(gameState->window, -1, SDL_RENDERER_ACCELERATED);
   gameState->screenSurface = SDL_GetWindowSurface(gameState->window);
   gameState->rectTexture = SDL_CreateTexture(gameState->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
   gameState->spriteTexture = loadTexture("media/smile.png", gameState->renderer);
   if (gameState->spriteTexture == NULL)
   {
      printf("Failed to load texture image!\n");
      return 1;
   }

   SDL_SetWindowOpacity(gameState->window, 0.5f);

   quit = game.start(&memory);

   while (quit == false)
   {
      quit = game.update(&memory, input);

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
            }
            break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
            {
               gameState->windowWidth = event.window.data1;
               gameState->windowHeight = event.window.data2;
               SDL_Log("Window %d size changed to %dx%d",
                       event.window.windowID, event.window.data1,
                       event.window.data2);
            }
            break;
            default:
               SDL_Log("Window %d got unknown event %d",
                       event.window.windowID, event.window.event);
               break;
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