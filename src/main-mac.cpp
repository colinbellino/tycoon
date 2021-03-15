#include <stdio.h>
#include <stdlib.h>

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
   MacGameCode game = {};
   bool quit = false;

#if HOT_RELOAD
   time_t now = time(0);
   time_t lastModified = now;
   time_t lastReload = now;

   printf("Starting game (with hot-reload).\n");
   game = loadGameCode();
#else
   game.start = gameStart;
   game.update = gameUpdate;
   game.isValid = true;
   printf("Starting game.\n");
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
      now = time(0);
      lastModified = getFileCreationTime(gamePath);
      // printTime("lastModified -> ", lastModified);
      // printTime("lastReload -> ", lastReload);
      // printTime("now -> ", now);

      if (lastModified > lastReload)
      {
         printf("Reloading game code.\n");
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

   printf("Exiting game.\n");
   printf("=================================\n");

   return 0;
}