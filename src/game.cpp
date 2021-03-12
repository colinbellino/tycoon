#include <stdio.h>

#include "game.h"

extern "C" GAME_UPDATE(gameUpdate)
{
    char str[] = "game.update()";
    printf("%s\n", str);

    return 0;
}