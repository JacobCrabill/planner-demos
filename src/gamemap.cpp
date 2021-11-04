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
            const olc::vf2d pos = vScreenPos - offset;
            if (pos.x + TW < 0 || pos.x >= pge->ScreenWidth() ||
                pos.y + TH < 0 || pos.y >= pge->ScreenHeight()) {
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

TileSet::TileSet(olc::PixelGameEngine* _pge, std::string fname, const int* typeMap, uint8_t nTypes) :
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
    for (int i = 0; i < nTypes; i++) {
        tiles[i].resize(3 * 7);
        const int IX = typeMap[i] * TS_NX;
        int n = 0;
        for (auto& spr : tiles[i]) {
            spr = new olc::Sprite(TS_W, TS_H);
            pge->SetDrawTarget(spr);
            const int ox = 32 * (IX + n % 3);
            const int oy = 32 * (n / 3);
            pge->DrawPartialSprite(0, 0, tileset, ox, oy, 32, 32);
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

int TileSet::GetRandomBaseTile()
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
    int r = rand() % wSum;
    for (int i = 0; i < 4; i++) {
        if (r < baseTileWgts[i]) {
            return baseTileIds[i];
        }
        r -= baseTileWgts[i];
    }

    assert("Should not reach here!");

    return baseTileIds[0];
}

olc::Sprite* TileSet::GetTileAt(uint8_t type, int idx)
{
    return tiles[type][idx];
}

int TileSet::GetIdxFromTopology(const std::vector<int>& topo)
{
    if (topo.size() == 0) return -1;

    return topoMap.at(topo);
}

void GameMap::GenerateMap()
{
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

    std::vector<int32_t> texmap(n_tiles);

    switch (config.mapType) {
        case MapType::PROCEDURAL: {
            #ifdef ENABLE_LIBNOISE
            /// Experimenting with Perlin noise from libnoise
            SetNoiseSeed(config.seed);
            for (int i = 0; i < ny; i++) {
                for (int j = 0; j < nx; j++) {
                    double x = (double)j  / (double)nx;
                    double y = (double)i  / (double)ny;
                    double val = GetNoise(5.*x, 5.*y); // Noise value in range [0, 1]
                    if (val <= .4) { val = 0; }
                    else if (val <= .7) { val = 1; }
                    else if (val <= .8) { val = 2; }
                    else if (val <= .9) { val = 2; }
                    else { val = 4; }

                    texmap[i*nx + j] = (int32_t)val;
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
                printf("Invalid map input - expected %d tiles, got %d\n", n_tiles, texmap.size());
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
        texmap[i] = std::min(std::max(0, texmap[i]), N_LAYERS - 1);
    }

    mapLoaded = true;
    map.resize(n_grid);
    for (int32_t i = 0; i < n_grid; i++) {
        const int32_t ix = i % dims.x; // x == column
        const int32_t iy = i / dims.x; // y == row
        const int32_t tidx = (ix + 1) + (iy + 1) * nx; // Index in full world input
        const int layer = texmap[tidx];
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

    // Use a neighborhood of 4 values to determine each tile's sprite
    // Note that the sprite will then be offset by 1/2 W, H in order for
    // the resultant terrain to line up with the given inputs
    for (int i = 0; i < n_grid; i++) {
        const int myL = map[i].layer;
        std::array<int, 4> bcs = {myL, myL, myL, myL};
        const int ix = map[i].vTileCoord.x;
        const int iy = map[i].vTileCoord.y;
    
        // Copy the neighborhood
        if (ix < nx && iy < ny) {
            bcs[0] = texmap[ix + (iy * nx)];
            bcs[1] = texmap[ix + 1 + (iy * nx)];
            bcs[2] = texmap[ix + 1 + (iy + 1) * nx];
            bcs[3] = texmap[ix + (iy + 1) * nx];
        }
        // Create our mapping for the multi-layer tile generation
        std::array<std::vector<int>, N_LAYERS> laymap;
        for (int L = 0; L < N_LAYERS; L++) {
            for (int b = 0; b < 4; b++) {
                if (bcs[b] == L) {
                    // If this entry is the current layer, put its index into the map
                    laymap[L].push_back(b);
                }
            }
        }

        olc::Sprite* tex = GetEdgeTileFor(laymap);
        map[i].dTexture = new olc::Decal(tex);
        pge->SetDrawTarget(nullptr);
        pge->DrawDecal({0.f, 0.f}, map[i].dTexture);
    }
};

olc::Sprite* GameMap::GetEdgeTileFor(std::array<std::vector<int>, N_LAYERS> laymap)
{
    // Returns a sprite created by layering the appropriate terrain types into
    // a single sprite, in order
  
    // For each available layer, map it's list of indices
    // to the matching tile index from its tileset
    // Default to -1 if that layer is unused
    std::array<int, N_LAYERS> tIdx;
    for (int L = 0; L < N_LAYERS; L++) {
        if (laymap[L].size() > 0) {
            tIdx[L] = tileSet->GetIdxFromTopology(laymap[L]);
        } else {
            tIdx[L] = -1;
        }
    }

    for (auto& t : tIdx) {
        // Add some spice - randomize all the 'plain' tiles
        if (t == tileSet->GetBaseIdx()) {
            t = tileSet->GetRandomBaseTile();
        }
    }

    olc::Sprite* spr = new olc::Sprite(TW, TH);
    pge->SetPixelMode(olc::Pixel::MASK);
    pge->SetDrawTarget(spr);

    for (uint8_t i = 0; i < N_LAYERS; i++) {
        if (tIdx[i] >= 0 && tIdx[i] < tileSet->TS_N_TILES) {
            pge->DrawSprite(0, 0, tileSet->GetTileAt(i, tIdx[i]));
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

