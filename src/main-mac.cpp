#include <stdio.h>
#include <SDL2/SDL.h>

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

#if HOT_RELOAD
const char *gamePath = "./build/game";

time_t getFileCreationTime(const char *filePath)
{
   struct stat attrib;
   stat(filePath, &attrib);
   return attrib.st_ctime;
}

void printTime(const char *prefix, time_t time)
{
   char date[18];
   strftime(date, 18, "%d-%m-%y %H:%M:%S", gmtime(&time));
   printf("%s%s\n", prefix, date);
   date[0] = 0;
}

GameCode loadGameCode()
{
   GameCode result = {};

   void *handle = dlopen(gamePath, RTLD_LAZY);
   if (!handle)
   {
      fprintf(stderr, "Error: %s\n", dlerror());
      return result;
   }

   result.handle = handle;
   result.start = (GameStart *)dlsym(handle, "gameStart");
   result.update = (GameUpdate *)dlsym(handle, "gameUpdate");
   result.isValid = (result.start != NULL && result.update != NULL);

   return result;
}

void unloadGameCode(GameCode *gameCode)
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
   GameCode game = {};
   GameMemory memory = {};
   bool quit = false;

#if HOT_RELOAD
   time_t lastModified = 0;
   time_t lastReload = 0;

   printf("Starting game (with hot-reload).\n");
   game = loadGameCode();
#else
   game.start = gameStart;
   game.update = gameUpdate;
   game.isValid = true;
   printf("Starting game.\n");
#endif

   quit = game.start();

   while (quit == false)
   {
      quit = game.update(&memory);

#if HOT_RELOAD
      time_t now = time(0);
      lastModified = getFileCreationTime(gamePath);
      // printTime("lastModified -> ", lastModified);
      // printTime("lastReload -> ", lastReload);
      // printTime("now -> ", now);

      if (lastModified > lastReload)
      {
         printf("Reloading game code.\n");
         unloadGameCode(&game);
         usleep(100 * 1000);
         game = loadGameCode();

         if (game.isValid == false)
         {
            quit = true;
         }

         lastReload = now;
      }
#endif
   }

   return 0;
}