#if !defined(MAIN_H)

#include <stdio.h>
#include <time.h>

#include "game.h"

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value) * 1024LL)
#define gigabytes(value) (megabytes(value) * 1024LL)
#define terabytes(value) (gigabytes(Value) * 1024LL)

#define print(...)       \
    printf(__VA_ARGS__); \
    fflush(stdout);

int gameStartStub(GameMemory *memory)
{
    printf("Using stub for game.start()\n");
    return 1;
}

int gameUpdateStub(GameMemory *memory)
{
    printf("Using stub for game.update()\n");
    return 1;
}

void printTime(const char *prefix, time_t time)
{
    char date[18];
    strftime(date, 18, "%d-%m-%y %H:%M:%S", gmtime(&time));
    printf("%s%s\n", prefix, date);
    date[0] = 0;
}

#define MAIN_H
#endif
