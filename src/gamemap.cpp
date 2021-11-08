/**
 * @File: gamemap.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#include "gamemap.hpp"

static std::array<olc::Pixel, 18> COLORS {
    olc::VERY_DARK_GREY, olc::VERY_DARK_RED, olc::VERY_DARK_YELLOW,
        olc::VERY_DARK_CYAN, olc::VERY_DARK_BLUE,
    olc::DARK_GREY, olc::DARK_RED, olc::DARK_YELLOW, olc::DARK_CYAN, olc::DARK_BLUE,
    olc::GREY, olc::RED, olc::YELLOW, olc::CYAN, olc::BLUE,
    olc::WHITE, olc::BLACK, olc::BLANK
};

void Tile::Draw(const olc::vi2d& offset)
{
    if (pge) {
        // Use the supplied texture if we have it
        if (dTexture) {
            // Only draw the tile if it's actually on the screen
            const olc::vf2d pos = vScreenPos - offset;
            if (pos.x + TW < 0 || pos.x >= (float)pge->ScreenWidth() ||
                pos.y + TH < 0 || pos.y >= (float)pge->ScreenHeight()) {
                return;
            }
            pge->DrawDecal(pos, dTexture);

        } else {
            // Otherwise draw a simple filled rectangle
            const int32_t x = vTileCoord.x * TW;
            const int32_t y = vTileCoord.y * TH;
            pge->FillRect(x, y, TW, TH, COLORS[layer]);
        }
    }
}

TileSet::TileSet(olc::PixelGameEngine* _pge, std::string fname, const uint8_t* typeMap, uint8_t nTypes) :
    pge(_pge),
    topoMap({
        {{0, 1, 2, 3}, 10},
        {{1, 2, 3},     5},
        {{0, 2, 3},     4},
        {{0, 1, 3},     1},
        {{0, 1, 2},     2},
        {{2, 3},        7},
        {{0, 3},       11},
        {{0, 1},       13},
        {{1, 2},        9},
        {{1, 3},       15},
        {{0, 2},       16},
        {{0},          14},
        {{1},          12},
        {{2},           6},
        {{3},           8}
    })
{
    // Load the full tileset
    // NOTE: Assuming single row of terrain tilesets for now
    tileset = new olc::Sprite(fname);

    // Load each individual terrain tile into its own sprite, for each terrain type
    tiles.resize(nTypes);
    for (uint8_t i = 0; i < nTypes; i++) {
        tiles[i].resize(3 * 7);
        const int IX = typeMap[i] * TS_NX;
        int n = 0;
        for (auto& spr : tiles[i]) {
            spr = new olc::Sprite(TS_W, TS_H);
            pge->SetDrawTarget(spr);
            const int ox = TW * (IX + n % 3);
            const int oy = TH * (n / 3);
            pge->DrawPartialSprite(0, 0, tileset, ox, oy, TW, TH);
            n++;
        }
    }
    pge->SetDrawTarget(nullptr);
}

TileSet::~TileSet()
{
    if (tileset != nullptr)
        delete tileset;
}

olc::Sprite* TileSet::GetBaseTile(uint8_t type)
{
    const int i = 1;
    const int j = 3;
    return tiles[type][3 * j + i];
}

int TileSet::GetRandomBaseTile(const float rval)
{
    // We have 1 plain tile and 3 'decorative' versions to choose from
    // Weight the plain one more so that it's not overwhelmingly decorative
    int baseTileIds[4] = {10, 18, 19, 20};
    int baseTileWgts[4] = {15, 1, 1, 1};

    int wSum = 0;
    for (int i = 0; i < 4; i++) {
        wSum += baseTileWgts[i];
    }

    // Use thresholding to assign our desired probabilities to the normal distribution
    int r;
    if (rval < 0) {
        r = rand() % wSum;
    } else {
        r = rval * wSum;
    }

    for (int i = 0; i < 4; i++) {
        if (r < baseTileWgts[i]) {
            return baseTileIds[i];
        }
        r -= baseTileWgts[i];
    }

    assert("Should not reach here!");

    return baseTileIds[0];
}

olc::Sprite* TileSet::GetDecorativeBaseTileFor(uint8_t type, uint8_t bt)
{
    const int baseTileIds[4] = {10, 18, 19, 20};
    int tidx = baseTileIds[bt];
    return GetTileAt(type, tidx);
}

olc::Sprite* TileSet::GetTileAt(uint8_t type, int idx)
{
    return tiles[type][idx];
}

int TileSet::GetIdxFromTopology(const std::vector<uint8_t>& topo)
{
    // See header file for detils of how topoMap was derived
    if (topo.size() == 0 || topo.size() > 4) return -1;

    return topoMap.at(topo);
}

void TextureCache::Setup(TileSet* tset)
{
    tileSet = tset;
    n_layers = tileSet->GetNTypes();

    // We need to at least pre-generate our collection of decorative base tiles
    baseTiles.resize(n_layers);

    const std::vector<int> btIds = tileSet->GetDecorativeBaseTilesIndices();
    for (uint8_t L = 0; L < n_layers; L++) {
        for (int bt : btIds) {
            baseTiles[L][bt] = new olc::Decal(tileSet->GetTileAt(L, bt));
        }
    }
}

bool TextureCache::Count(const std::array<uint8_t, 4>& bcs)
{
    return cache.count(bcs) || IsBaseTile(bcs);
}

bool TextureCache::IsBaseTile(const std::array<uint8_t, 4>& bcs)
{
    const int bc0 = bcs[0];
    for (uint8_t i = 1; i < 4; i++) {
        if (bcs[i] != bc0) {
            return false;
        }
    }

    return true;
}
olc::Decal* TextureCache::Get(const std::array<uint8_t, 4>& bcs, int ix, int iy)
{
    /// TODO: I think this whole cache-with-randomness thing is overly convoluted.  Rethink later.
    if (IsBaseTile(bcs)) {
        // Use a nice, deterministic "random number" to get the "random" tile
        float rval = SimpleRand(ix, iy);
        // float rval = GetNoise((ix % 256) / 256., (iy % 256) / 256.);
        int t = tileSet->GetRandomBaseTile(rval); //GetNoise(50.*ix, 50.*iy));
        return baseTiles[bcs[0]].at(t);
    }

    // Note: This should throw an error if we improperly try to get a tile that doesn't exist
    return cache.at(bcs);
};

void TextureCache::Set(const std::array<uint8_t, 4>& bcs, olc::Decal* dec)
{
    if (IsBaseTile(bcs)) return;

    cache[bcs] = dec;
}

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
     * - Cache previously-generated sprite data (up to some limit...?)
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

    texCache.Setup(tileSet);

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
            float sum = std::reduce(config.terrainWeights.begin(), config.terrainWeights.end());
            for (auto &w : config.terrainWeights) {
                w /= sum;
            }

            // Compute the cumulative sums.  These are used to split the range [0, 1] into regions.
            std::vector<float> wsum = config.terrainWeights;
            for (uint32_t i = 1; i < wsum.size(); i++) {
                wsum[i] += wsum[i - 1];
            }
            
            /// Experimenting with Perlin noise from libnoise
            SetNoiseSeed(config.noiseSeed);
            for (int i = 0; i < ny; i++) {
                for (int j = 0; j < nx; j++) {
                    double x = (double)j  / (double)nx;
                    double y = (double)i  / (double)ny;
                    double val = GetNoise(config.noiseScale*x, config.noiseScale*y); // Noise value in range [0, 1]
                    if (val <= wsum[0]) { val = 0; }
                    else if (val <= wsum[1]) { val = 1; }
                    else if (val <= wsum[2]) { val = 2; }
                    else if (val <= wsum[3]) { val = 3; }
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
    map.resize(n_grid);
    for (int32_t i = 0; i < n_grid; i++) {
        const int32_t ix = i % dims.x; // x == column
        const int32_t iy = i / dims.x; // y == row
        const int32_t tidx = (ix + 1) + (iy + 1) * nx; // Index in full world input
        const uint8_t layer = texmap[tidx];
        const TERRAIN_TYPE tt = static_cast<TERRAIN_TYPE>(layers[layer]);
        map[i].layer = layer;
        map[i].fEffort = teffort.at(tt);

        // Note:
        // With how we're currently creating the terrain, we need to offset
        // the sprites by half a tile size for this to actually work
        map[i].vTileCoord = {ix, iy};
        map[i].vScreenPos = {(float)(ix * TW - TW/2), float(iy * TH - TH/2)};
        map[i].pge = pge;
    }

    // Use a neighborhood of 4 values to determine each tile's sprite (See header file)
    // Note that the sprite will then be offset by 1/2 W, H in order for
    // the resultant terrain to line up with the given inputs
    for (int i = 0; i < n_grid; i++) {
        const uint8_t myL = map[i].layer;
        std::array<uint8_t, 4> bcs = {myL, myL, myL, myL};
        const int ix = map[i].vTileCoord.x;
        const int iy = map[i].vTileCoord.y;

        // Copy the neighborhood
        if (ix < nx && iy < ny) {
            bcs[0] = texmap[ix + (iy * nx)];
            bcs[1] = texmap[ix + 1 + (iy * nx)];
            bcs[2] = texmap[ix + 1 + (iy + 1) * nx];
            bcs[3] = texmap[ix + (iy + 1) * nx];
        }

        // Check our texture cache
        // TODO: How do we now add "spice" to the plain parts of the map?
        //       (Swap out plain base tiles with random decorative variants)
        if (!texCache.Count(bcs)) {
            // Create our mapping for the multi-layer tile generation
            std::array<std::vector<uint8_t>, N_LAYERS> laymap;
            for (uint8_t L = 0; L < N_LAYERS; L++) {
                for (uint8_t b = 0; b < 4; b++) {
                    if (bcs[b] == L) {
                        // If this entry is the current layer, put its index into the map
                        laymap[L].push_back(b);
                    }
                }
            }

            texCache.Set(bcs, new olc::Decal(GetEdgeTileFor(laymap)));
        }
        map[i].dTexture = texCache.Get(bcs, ix, iy);
    }
};

olc::Sprite* GameMap::GetEdgeTileFor(std::array<std::vector<uint8_t>, N_LAYERS> laymap)
{
    // Returns a sprite created by layering the appropriate terrain types into
    // a single sprite, in order
    PROFILE_FUNC();
  
    // For each available layer, map its list of indices
    // to the matching tile index from its tileset
    // Default to -1 if that layer is unused
    std::array<int, N_LAYERS> tIdx;
    for (uint8_t L = 0; L < N_LAYERS; L++) {
        if (laymap[L].size() > 0) {
            tIdx[L] = tileSet->GetIdxFromTopology(laymap[L]);
        } else {
            tIdx[L] = -1;
        }
    }

    for (auto& t : tIdx) {
        // Add some spice - randomize all the 'plain' tiles
        /// TODO: This will have to be changed once we start adding/removing tiles dynamically
        if (t == tileSet->GetBaseIdx()) {
            t = tileSet->GetRandomBaseTile();
        }
    }

    olc::Sprite* spr = new olc::Sprite(TW, TH);
    pge->SetPixelMode(olc::Pixel::MASK);
    pge->SetDrawTarget(spr);

    for (uint8_t L = 0; L < N_LAYERS; L++) {
        if (tIdx[L] >= 0 && tIdx[L] < tileSet->TS_N_TILES) {
            pge->DrawSprite(0, 0, tileSet->GetTileAt(L, tIdx[L]));
        }
    }

    pge->SetDrawTarget(nullptr);

    return spr;
}

TERRAIN_TYPE GameMap::GetTerrainAt(int ix, int iy)
{
    const int idx = ix + iy * dims.x;
    return GetTerrainAt(idx);
}

TERRAIN_TYPE GameMap::GetTerrainAt(int idx)
{
    if (idx < 0 || (size_t)idx >= map.size()) return TYPE_COUNT;

    return static_cast<TERRAIN_TYPE>(layers[map[idx].layer]);
}

float GameMap::GetEffortAt(int ix, int iy)
{
    const int idx = ix + iy * dims.x;
    return GetEffortAt(idx);
}

float GameMap::GetEffortAt(int idx)
{
    if (idx < 0 || (size_t)idx >= map.size()) return -1.f;

    return map[idx].fEffort;
}

void GameMap::Draw(const olc::vi2d& offset)
{
    for (auto &T : map) {
        T.Draw(offset);
    }
};

