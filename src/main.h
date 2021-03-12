#if !defined(MAIN_H)

#include <stdio.h>

#include "game.h"

GAME_UPDATE(gameUpdateStub)
{
    printf("Using stub for game.update()\n");
    return 1;
}

typedef struct
{
    void *handle;
    GameUpdate *update;
    // GameInit *init;
    bool isValid;
} GameCode;

#define MAIN_H
#endif
