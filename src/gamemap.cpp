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

void Tile::Draw()
{
    if (pge) {
        // Use the supplied texture if we have it
        if (dTexture) {
            pge->DrawDecal(vScreenPos, dTexture);

        } else {
            // Otherwise draw a simple filled rectangle
            const int32_t x = vTileCoord.x * W;
            const int32_t y = vTileCoord.y * H;
            pge->FillRect(x, y, W, H, COLORS[layer]);
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
        const int IX = typeMap[i] * NX;
        int n = 0;
        for (auto& spr : tiles[i]) {
            spr = new olc::Sprite(TW, TH);
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

GameMap::GameMap()
{

}

void GameMap::GenerateMap()
{
    // Load the terrain-tile assets
    tileSet = new TileSet(pge, "resources/lpc-terrains/reduced-tileset-1.png", layers, N_LAYERS);

    // Load the map definition file
    std::string options[3] = {"test.dat", "test-terrain-2.dat", "test-terrain-8x4.dat"};
    std::ifstream f(options[0], std::ifstream::in);
    
    int32_t nx, ny;
    f >> nx >> ny;
    int32_t n_tiles = nx * ny;
    int32_t n_grid = (nx - 1) * (ny - 1);

    dims.x = nx - 1;
    dims.y = ny - 1;

    std::vector<int32_t> texmap(n_tiles);
    int32_t n_read = 0;
    while (f >> texmap[n_read]) n_read++;

    // Constrain the inputs to be within our layer definitions
    for (int i = 0; i < texmap.size(); i++) {
        texmap[i] = std::min(std::max(0, texmap[i]), N_LAYERS - 1);
        printf("%d ", texmap[i]);
        if ((i+1) % nx == 0) printf("\n");
    }

    if (n_read < n_tiles) {
        std::cout << "Error while reading texture map - unexpected EOF";
        std::cout << std::endl;
        mapLoaded = false;
        return;
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
        map[i].vScreenPos = {ix * 32.f - 16.f, iy * 32.f - 16.f};
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

    olc::Sprite* spr = new olc::Sprite(32, 32);
    pge->SetPixelMode(olc::Pixel::MASK);
    pge->SetDrawTarget(spr);

    for (int i = 0; i < N_LAYERS; i++) {
        if (tIdx[i] >= 0 && tIdx[i] < tileSet->N_TILES) {
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
    if (idx < 0 || idx >= map.size()) return TYPE_COUNT;

    return static_cast<TERRAIN_TYPE>(layers[map[idx].layer]);
}

float GameMap::GetEffortAt(int ix, int iy)
{
    const int idx = ix + iy * dims.x;
    return GetEffortAt(idx);
}

float GameMap::GetEffortAt(int idx)
{
    if (idx < 0 || idx >= map.size()) return -1.f;

    return map[idx].fEffort;
}

void GameMap::Draw()
{
    for (auto &T : map) {
        T.Draw();
    }
};

