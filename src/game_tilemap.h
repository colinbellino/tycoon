#include <math.h>

// struct TileMap_position
// {
//     // NOTE(casey): These are fixed point tile locations.  The high
//     // bits are the tile chunk index, and the low bits are the tile
//     // index in the chunk.
//     uint32_t absoluteTileX;
//     uint32_t absoluteTileY;
//     uint32_t absoluteTileZ;

//     // NOTE(casey): These are the offsets from the tile center
//     float OffsetX;
//     float OffsetY;
// };

struct TileChunkPosition
{
    uint32_t tileChunkX;
    uint32_t tileChunkY;
    uint32_t tileChunkZ;

    uint32_t relTileX;
    uint32_t relTileY;
};

struct Tile
{
    uint32_t value;
};

struct TileChunk
{
    Tile *tiles;
};

struct TileMap
{
    uint32_t chunkShift;
    uint32_t chunkMask;
    uint32_t chunkDimension;

    float tileSizeInMeters;

    uint32_t tileChunkCountX;
    uint32_t tileChunkCountY;
    uint32_t tileChunkCountZ;
    TileChunk *tileChunks;
};

TileChunk *getTileChunk(TileMap *tileMap, uint32_t tileChunkX, uint32_t tileChunkY, uint32_t tileChunkZ)
{
    TileChunk *chunk = 0;

    if ((tileChunkX >= 0) && (tileChunkX < tileMap->tileChunkCountX) &&
        (tileChunkY >= 0) && (tileChunkY < tileMap->tileChunkCountY) &&
        (tileChunkZ >= 0) && (tileChunkZ < tileMap->tileChunkCountZ))
    {
        int index = tileChunkZ * tileMap->tileChunkCountY * tileMap->tileChunkCountX + tileChunkY * tileMap->tileChunkCountX + tileChunkX;
        chunk = &tileMap->tileChunks[index];
    }

    return (chunk);
}

TileChunkPosition getChunkPositionFor(TileMap *tileMap, uint32_t absoluteTileX, uint32_t absoluteTileY, uint32_t absoluteTileZ)
{
    TileChunkPosition position;

    position.tileChunkX = absoluteTileX >> tileMap->chunkShift;
    position.tileChunkY = absoluteTileY >> tileMap->chunkShift;
    position.tileChunkZ = absoluteTileZ;
    position.relTileX = absoluteTileX & tileMap->chunkMask;
    position.relTileY = absoluteTileY & tileMap->chunkMask;

    return position;
}

Tile getTileValueUnchecked(TileMap *tileMap, TileChunk *tileChunk, uint32_t tileX, uint32_t tileY)
{
    assert(tileChunk, "Invalid chunk.");
    assert(tileX < tileMap->chunkDimension, "Out of bound (x).");
    assert(tileY < tileMap->chunkDimension, "Out of bound (y).");

    return tileChunk->tiles[tileY * tileMap->chunkDimension + tileX];
}

void setTileValueUnchecked(TileMap *tileMap, TileChunk *tileChunk, uint32_t tileX, uint32_t tileY, Tile tileValue)
{
    assert(tileChunk, "Invalid chunk.");
    assert(tileX < tileMap->chunkDimension, "Out of bound (x).");
    assert(tileY < tileMap->chunkDimension, "Out of bound (y).");

    tileChunk->tiles[tileY * tileMap->chunkDimension + tileX] = tileValue;
}

Tile getTileValue(TileMap *tileMap, TileChunk *tileChunk, uint32_t testTileX, uint32_t testTileY)
{
    Tile tile;

    if (tileChunk && tileChunk->tiles)
    {
        tile = getTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY);
    }

    return tile;
}

Tile getTileValue(TileMap *tileMap, uint32_t absoluteTileX, uint32_t absoluteTileY, uint32_t absoluteTileZ)
{
    TileChunkPosition chunkPosition = getChunkPositionFor(tileMap, absoluteTileX, absoluteTileY, absoluteTileZ);
    TileChunk *tileChunk = getTileChunk(tileMap, chunkPosition.tileChunkX, chunkPosition.tileChunkY, chunkPosition.tileChunkZ);

    return getTileValue(tileMap, tileChunk, chunkPosition.relTileX, chunkPosition.relTileY);
}

void setTileValue(TileMap *tileMap, TileChunk *tileChunk,
                  uint32_t testTileX, uint32_t testTileY, Tile tileValue)
{
    if (tileChunk && tileChunk->tiles)
    {
        setTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY, tileValue);
    }
}

void setTileValue(
    MemoryArena *arena, TileMap *tileMap,
    uint32_t absoluteTileX, uint32_t absoluteTileY, uint32_t absoluteTileZ,
    Tile tileValue)
{
    TileChunkPosition chunkPosition = getChunkPositionFor(tileMap, absoluteTileX, absoluteTileY, absoluteTileZ);
    TileChunk *tileChunk = getTileChunk(tileMap, chunkPosition.tileChunkX, chunkPosition.tileChunkY, chunkPosition.tileChunkZ);

    assert(tileChunk, "Couldn't find tile chunk.");
    if (!tileChunk->tiles)
    {
        uint32_t tileCount = tileMap->chunkDimension * tileMap->chunkDimension;
        tileChunk->tiles = pushArray(arena, tileCount, Tile);
        for (uint32_t TileIndex = 0; TileIndex < tileCount; ++TileIndex)
        {
            tileChunk->tiles[TileIndex] = tileValue;
        }
    }

    setTileValue(tileMap, tileChunk, chunkPosition.relTileX, chunkPosition.relTileY, tileValue);
}