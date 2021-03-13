#if !defined(MAIN_H)

#include <stdio.h>

#include "game.h"

GAME_START(gameStartStub)
{
    printf("Using stub for game.start()\n");
    return 1;
}

GAME_UPDATE(gameUpdateStub)
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

#define MAIN_H
#endif
