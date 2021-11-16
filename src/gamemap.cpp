/**
 * @File: gamemap.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#include "gamemap.hpp"

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
     * - Generate only the sprites we need to display
     * - Create/Delete tiles as the map scrolls
     *   - Tiles will no longer be in a flattened 2D array; will need to push/pop tiles
     *     from a generic container and use an index map to look up a row,col
     * - Use chunks of, say, 16x16 or 32x32 tiles and add/delete those as the map scrolls
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

    dims.x = nx - 1;
    dims.y = ny - 1;

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
            for (int i = 0; i < ny; i++) {
                for (int j = 0; j < nx; j++) {
                    double x = (double)j  / (double)nx;
                    double y = (double)i  / (double)ny;
                    double val = GetNoise(config.noiseScale*x, config.noiseScale*y); // Noise value in range [0, 1]
                    if (val <= tRangeSums[0]) { val = 0; }
                    else if (val <= tRangeSums[1]) { val = 1; }
                    else if (val <= tRangeSums[2]) { val = 2; }
                    else if (val <= tRangeSums[3]) { val = 3; }
                    else { val = 4; }

                    texmap[i*nx + j] = (uint8_t)val;
                }
            }
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
            break;
        }

        default:
            printf("Unknown MapType option '%d'- expecting STATIC or PROCEDURAL\n", config.mapType);
            exit(1);
            break;
    }

    // Constrain the inputs to be within our layer definitions
    for (uint32_t i = 0; i < texmap.size(); i++) {
        texmap[i] = (uint8_t)std::min(std::max(0, (int)texmap[i]), N_LAYERS - 1);
    }

    mapLoaded = true;
    for (int32_t i = 0; i < n_grid; i++) {
        const int32_t ix = i % dims.x; // x == column
        const int32_t iy = i / dims.x; // y == row
        const int32_t tidx = (ix + 1) + (iy + 1) * nx; // Index in full world input
        const uint8_t layer = texmap[tidx];
        const TERRAIN_TYPE tt = static_cast<TERRAIN_TYPE>(layers[layer]);
        map[{ix, iy}].layer = layer;
        map[{ix, iy}].fEffort = teffort.at(tt);

        // Note:
        // With how we're currently creating the terrain, we need to offset
        // the sprites by half a tile size for this to actually work
        map[{ix, iy}].vTileCoord = {ix, iy};
        map[{ix, iy}].vScreenPos = {(float)(ix * TW - TW/2), float(iy * TH - TH/2)};
        map[{ix, iy}].pge = pge;
    }

    // Use a neighborhood of 4 values to determine each tile's sprite (See header file)
    // Note that the sprite will then be offset by 1/2 W, H in order for
    // the resultant terrain to line up with the given inputs
    for (auto& tile : map) {
        const uint8_t myL = tile.second.layer;
        std::array<uint8_t, 4> bcs = {myL, myL, myL, myL};
        const int ix = tile.first.x;
        const int iy = tile.first.y;

        // Copy the neighborhood
        if (ix < nx && iy < ny) {
            bcs[0] = texmap[ix + (iy * nx)];
            bcs[1] = texmap[ix + 1 + (iy * nx)];
            bcs[2] = texmap[ix + 1 + (iy + 1) * nx];
            bcs[3] = texmap[ix + (iy + 1) * nx];
        }

        tile.second.dTexture = tileSet->GetTextureFor(bcs, {ix, iy});
    }
};

TERRAIN_TYPE GameMap::GetTerrainAt(int ix, int iy)
{
    if (!map.count({ix, iy})) return TYPE_COUNT;

    return static_cast<TERRAIN_TYPE>(layers[map[{ix, iy}].layer]);
}

float GameMap::GetEffortAt(int ix, int iy)
{
    if (!map.count({ix, iy})) return -1.f;

    return map[{ix, iy}].fEffort;
}

void GameMap::Draw(const olc::vi2d& offset)
{
    int ix = offset.x / TW;
    int iy = offset.y / TH;
    if (ix != idxTL.x || iy != idxTL.y) {
        // printf("Update screen!\n");
        /// TODO:
        /// Remove "dead" tiles
        /// Add new tiles
        idxTL = {ix, iy};
    }
    for (auto &T : map) {
        T.second.Draw(offset);
    }
};

