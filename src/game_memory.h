#if !defined(MEMORY_H)

#include <math.h>
#include <stdio.h>

#include "game_utils.h"

struct MemoryArena
{
    uint8_t *base;
    size_t size;
    size_t used;
};

void initializeArena(MemoryArena *arena, uint8_t *base, size_t size)
{
    arena->base = base;
    arena->size = size;
    arena->used = 0;
}

#define pushStruct(arena, type) (type *)pushSize(arena, sizeof(type))

#define pushArray(arena, count, type) (type *)pushSize(arena, (count) * sizeof(type))

void *pushSize(MemoryArena *arena, size_t size)
{
    assert(arena->used + size <= arena->size, "Area size to large.");

    void *Result = arena->base + arena->used;
    arena->used += size;

    return Result;
}

#define MEMORY_H
#endif
