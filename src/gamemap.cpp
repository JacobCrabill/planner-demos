/**
 * @File: gamemap.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#include "gamemap.hpp"

#include <set>

GameMap::~GameMap()
{
    if (tileSet) {
        delete tileSet;
    }
}

void GameMap::GenerateMap()
{
    /**
     * TODO:
     *
     * Right now, we're generating the _entire_ map, which includes:
     * - A sprite for every single tile
     * - Building said sprite from each tile's boundary conditions (BCs)
     * - No caching of duplicate sprites or boundary conditions
     *
     * In order to scale to an "infinite" world, we must change how we do things:
     * - **(DONE)** Display only the sprites within the screen area
     * - **(DONE)** Generate only the sprites we need to display
     * - **(DONE)** Create/Delete tiles as the map scrolls
     *   - Tiles will no longer be in a flattened 2D array; will need to push/pop tiles
     *     from a generic container and use an index map to look up a row,col
     * - **(DONE)** Use chunks of, say, 16x16 or 32x32 tiles and add/delete those as the map scrolls
     * - **(DONE)** Cache previously-generated sprite data (up to some limit...?)
     *   - Specifically, map the layers/BCs to a pre-generated sprite, and reuse that
     *     for any other tiles that have the same layer BCs
     * - Optimize the sprite generation...
     */
    PROFILE_FUNC();

    // Load the terrain-tile assets
    if (tileSet) {
        delete tileSet;
        tileSet = nullptr;
    }

    tileSet = new TileSet(pge, "resources/lpc-terrains/reduced-tileset-1.png", layers, N_LAYERS);

    // Load / Create the map definition

    int32_t nx = config.dims.x;
    int32_t ny = config.dims.y;
    int32_t n_tiles = nx * ny;
    int32_t n_grid = (nx - 1) * (ny - 1);

    dims.x = nx;
    dims.y = ny;

    std::vector<uint8_t> texmap(n_tiles);

    switch (config.mapType) {
        case MapType::PROCEDURAL: {
#ifdef ENABLE_LIBNOISE
            PROFILE("Perlin MapGen");
            // Configure the relative amounts of each terrain type
            if (config.terrainWeights.size() != N_LAYERS) {
                printf("ERROR: Incorrect number of terrain weights given (expected %d, got %lu)\n", N_LAYERS, config.terrainWeights.size());
                config.terrainWeights.assign(N_LAYERS, 1.f);
            }

            // Normalize the total amount to 1
            float sum = 0;
            for (auto w : config.terrainWeights) {
                sum += w;
            }
            for (auto &w : config.terrainWeights) {
                w /= sum;
            }

            // Compute the cumulative sums.  These are used to split the range [0, 1] into regions.
            tRangeSums = config.terrainWeights;
            for (uint32_t i = 1; i < tRangeSums.size(); i++) {
                tRangeSums[i] += tRangeSums[i - 1];
            }

            /// Experimenting with Perlin noise from libnoise
            SetNoiseSeed(config.noiseSeed);
            for (int j = 0; j < ny; j++) {
                for (int i = 0; i < nx; i++) {
                    TERRAIN_TYPE val = GetTerrainAt(i, j);

                    texmap[j*nx + i] = (uint8_t)val;
                }
            }

            olc::vi2d nchunks = {pge->ScreenWidth()/TH/ChunkSize.x + 3, pge->ScreenHeight()/TH/ChunkSize.y + 3};
            for (int j = -1; j < nchunks.y - 1; j++) {
                for (int i = -1; i < nchunks.x - 1; i++) {
                    olc::vi2d start = {ChunkSize.x*i, ChunkSize.y*j};
                    if (start.x >= dims.x || start.y >= dims.y)
                        continue;

                    AddChunk(start, ChunkSize);
                }
            }

            chidTL = -ChunkSize;
            chidBR = (nchunks - olc::vi2d({1,1})) * ChunkSize;
#else
            printf("Unable to generate procedural map without libnoise\nSet ENABLE_LIBNOISE to build");
            exit(1);
#endif
            break;
        }

        case MapType::STATIC: {
            texmap = config.map;
            if (texmap.size() != (size_t)n_tiles) {
                printf("Invalid map input - expected %d tiles, got %lu\n", n_tiles, texmap.size());
                exit(1);
            }

            // Constrain the inputs to be within our layer definitions
            for (uint32_t i = 0; i < texmap.size(); i++) {
                texmap[i] = (uint8_t)std::min(std::max(0, (int)texmap[i]), N_LAYERS - 1);
            }

            for (int j = 0; j < pge->ScreenHeight()/ChunkSize.y + 1; j++) {
                for (int i = 0; i < pge->ScreenWidth()/ChunkSize.x + 1; i++) {
                    olc::vi2d start = {ChunkSize.x*i, ChunkSize.y*j};
                    if (start.x > dims.x || start.y > dims.y)
                        continue;

                    AddChunk(start, ChunkSize);
                }
            }

            break;
        }

        default:
            printf("Unknown MapType option '%d'- expecting STATIC or PROCEDURAL\n", config.mapType);
            exit(1);
            break;
    }
};

void GameMap::AddChunk(olc::vi2d start, olc::vi2d size)
{
    if (chunks.count(start)) return;

    auto& chunk = chunks[start];
    chunk.coord = start;
    chunk.dims = size;
    chunk.tiles.resize(size.x * size.y);

    // First, assign the terrain type to each tile
    for (int j = 0; j < size.y; j++) {
        const int iy = start.y + j;
        for (int i = 0; i < size.x; i++) {
            const int ix = start.x + i;
            const uint8_t layer = GetLayerAt(ix, iy);
            const TERRAIN_TYPE tt = (TERRAIN_TYPE)layers[layer];

            Tile& tile = chunk.tiles[j*size.x + i];
            tile.layer = layer;
            tile.fEffort = teffort.at(tt);

            // Note:
            // With how we're currently creating the terrain, we need to offset
            // the sprites by half a tile size for this to actually work
            tile.vTileCoord = {ix, iy};
            tile.vScreenPos = {(float)(ix * TW - TW/2), float(iy * TH - TH/2)};
            tile.pge = pge;
        }
    }

    // Next, apply the correct texture for each tile
    for (int j = 0; j < size.y; j++) {
        for (int i = 0; i < size.x; i++) {
            Tile& tile = chunk.tiles[j*size.x + i];
            const uint8_t myL = tile.layer;
            std::array<uint8_t, 4> bcs = {myL, myL, myL, myL};
            const int ix = tile.vTileCoord.x;
            const int iy = tile.vTileCoord.y;

            // Copy the neighborhood
            bcs[0] = GetLayerAt(ix - 1, iy - 1);
            bcs[1] = GetLayerAt(ix, iy - 1);
            bcs[2] = GetLayerAt(ix, iy);
            bcs[3] = GetLayerAt(ix - 1, iy);

            tile.dTexture = tileSet->GetTextureFor(bcs, tile.vTileCoord);
        }
    }
}

void GameMap::RemoveChunk(olc::vi2d start)
{
    if (!chunks.count(start)) return;

    chunks.erase(start);
}

uint8_t GameMap::GetLayerAt(int ix, int iy)
{
    // Not in an existing chunk; calculate it, look it up, or
    if (config.mapType == MapType::STATIC) {
        if (ix >= 0 && ix < dims.x && iy >= 0 && iy < dims.y) {
            return config.map[iy*dims.x + ix];
        }
        return N_LAYERS;

    } else {
        double x = (double)iy  / (double)config.dims.x;
        double y = (double)ix  / (double)config.dims.y;
        double val = GetNoise(config.noiseScale*x, config.noiseScale*y); // Noise value in range [0, 1]
        if (val <= tRangeSums[0]) { val = 0; }
        else if (val <= tRangeSums[1]) { val = 1; }
        else if (val <= tRangeSums[2]) { val = 2; }
        else if (val <= tRangeSums[3]) { val = 3; }
        else { val = 4; }

        return (uint8_t)val;
    }
}

TERRAIN_TYPE GameMap::GetTerrainAt(int ix, int iy)
{
    return (TERRAIN_TYPE)layers[GetLayerAt(ix, iy)];
}

float GameMap::GetEffortAt(int ix, int iy)
{
    for (const auto& entry : chunks) {
        auto& chunk = entry.second;
        if (ix >= chunk.coord.x && ix < chunk.coord.x + chunk.dims.x &&
            iy >= chunk.coord.y && iy < chunk.coord.y + chunk.dims.y) {
                int i = ix - chunk.coord.x;
                int j = iy - chunk.coord.y;
                return chunk.tiles[j*chunk.dims.x + i].fEffort;
            }
    }

    return -1.f;
}

void GameMap::Draw(const olc::vi2d& offset)
{
    for (auto& entry : chunks) {
        for (auto& tile : entry.second.tiles) {
            tile.Draw(offset);
        }
    }

    olc::vi2d new_idxTL = offset / olc::vi2d({TW, TH});
    olc::vi2d new_idxBR = new_idxTL + olc::vi2d({(pge->ScreenWidth() + TW/2) / TW, (pge->ScreenHeight() + TH/2) / TH});

    if (new_idxTL.x != idxTL.x || new_idxTL.y != idxTL.y) {
        // Remove "dead" chunks, add new chunks
        const olc::vi2d nchunks = {pge->ScreenWidth()/TH/ChunkSize.x + 3, pge->ScreenHeight()/TH/ChunkSize.y + 3};
        const olc::vi2d new_chidTL = (idxTL / ChunkSize) * ChunkSize - ChunkSize; // Integer multiples of ChunkSize
        const olc::vi2d new_chidBR = new_chidTL + ChunkSize * nchunks;

        if (new_chidTL != chidTL) {
            // std::cout << "Old chunk extents: " << chidTL << " -> " << chidBR << std::endl;
            // std::cout << "New chunk extents: " << new_chidTL << " -> " << new_chidBR << std::endl;

            std::set<olc::vi2d> desired_chids;
            for (int i = 0; i < nchunks.x; i++) {
                for (int j = 0; j < nchunks.y; j++) {
                    desired_chids.insert({new_chidTL.x + i*ChunkSize.x, new_chidTL.y + j*ChunkSize.y});
                }
            }

            std::set<olc::vi2d> remove_chids;
            for (auto& entry : chunks) {
                if (!desired_chids.count(entry.first)) {
                    remove_chids.insert(entry.first);
                }
            }
            for (auto chid : remove_chids) {
                RemoveChunk(chid);
            }

            for (auto chid : desired_chids) {
                AddChunk(chid, ChunkSize);
            }
        }

        idxTL = new_idxTL;
        idxBR = new_idxBR;
        chidTL = new_chidTL;
        chidBR = new_chidBR;
    }
};

