#if !defined(MAIN_H)

#include <stdio.h>

#include "game.h"

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

typedef struct
{
    void *handle;
    GameStart *start;
    GameUpdate *update;
    bool isValid;
} GameCode;

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value) * 1024LL)
#define gigabytes(value) (megabytes(value) * 1024LL)
#define terabytes(value) (gigabytes(Value) * 1024LL)

#define MAIN_H
#endif
