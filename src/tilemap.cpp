/**
 * @File: tilemap.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#include "tilemap.hpp"

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
            pge->FillRect(x, y, W, H, pColor);
        }
    }
}

TileSet::TileSet(olc::PixelGameEngine* pge, olc::Sprite* sprMap, int tIdx) :
    _pge(pge)
{
    // Load the full tileset for our terrain type
    tileset = new olc::Sprite(TW, TH);
    _pge->SetDrawTarget(tileset);
    // NOTE: Assuming single row of terrain tilesets for now
    _pge->DrawPartialSprite(0, 0, sprMap, tIdx * TW, 0 * TH, TW, TH);
    _pge->SetDrawTarget(nullptr);

    // Load each individual terrain tile into its own sprite
    tiles.resize(3 * 7);
    int n = 0;
    for (auto& spr : tiles) {
        spr = new olc::Sprite(TW, TH);
        _pge->SetDrawTarget(spr);
        const int ox = 32 * (n % 3);
        const int oy = 32 * (n / 3);
        _pge->DrawPartialSprite(0, 0, tileset, ox, oy, 32, 32);
        n++;
    }
    _pge->SetDrawTarget(nullptr);
}

TileSet::~TileSet()
{
    if (tileset != nullptr)
        delete tileset;
}

void TileSet::DrawSingleTile(const olc::vi2d sLoc, const olc::vi2d tIdx)
{
    if (tIdx.x > 2 || tIdx.y > 6) return;

    // Draw tile [tIdx] at [sLoc] on the screen
    _pge->DrawSprite(sLoc.x, sLoc.y, tileset);
}

olc::Sprite* TileSet::GetBaseTile()
{
    const int i = 1;
    const int j = 3;
    return tiles[3 * j + i];
}

olc::Sprite* TileSet::GetTileAt(int idx)
{
    return tiles[idx];
}

void TileSet::DrawBaseTile(const olc::vi2d sLoc)
{
    const int i = 1;
    const int j = 3;
    _pge->DrawPartialSprite(sLoc.x, sLoc.y, tileset, i, j, 32, 32);
}

TileMap::TileMap()
{
    /**
     * We are using an offset grid to allow finer control of the terrain
     * stitching _with our given tileset_
     *
     * e.g.: The input map of tile terrain values is offset by 1/2 of a tile
     * width/height such that each displayed tile determines its final value
     * from the 'corner' between 4 input values
     *
     * X X = X: Base Tile (10, or decorative)
     * X X = O: N/A
     *
     * O X = X: Top-Left Cutout (5)
     * X X = O: Bottom-Right Overlay (14)
     *
     * x O = X: Top-Right Cutout (4)
     * X X = O: Bottom-Left Overlay (12)
     *
     * X X = X: Bottom-Right Cutout (1)
     * X O = O: Top-Left Overlay (6)
     *
     * X X = Bottom-Left Cutout (2)
     * O X = Top-Right Overlay (8)
     *
     * O O = X: Top Center Overlay (7)
     * X X = O: Bottom Center Overlay (13)
     *
     * X O = X: Right Center Overlay (11)
     * X O = O: Left Center Overlay (9)
     *
     * X X = X: Bottom Center Overlay (13)
     * O O = O: Top Center Overlay (7)
     *
     * O X = X: Left Center Overlay (9)
     * O X = O: Right Center Overlay (11)
     *
     * O X = X: BL/TR Diag (15)
     * X O = O: TL/BR Diat (16)
     *
     * X O = X: TL/BR Diat (16)
     * O X = O: BL/TR Diag (15)
     *
     * X O = X: Bottom-Right Overlay (14)
     * O O = O: Top-Left Cutout (5)
     *
     * O X = X: Bottom-Left Overlay (12)
     * O O = O: Top-Right Cutout (4)
     *
     * O O = X: Top-Left Overlay (6)
     * O X = O: Bottom-Right Cutout (1)
     *
     * O O = X: Top-Right Overlay (8)
     * X O = O: Bottom-Left Cutout (2)
     *
     * X X = X: N/A
     * X X = O: Base Tile (10, or decorative)
     */

    // Map each possible boundary condition to an overlay tile
    // topoMap[{1, 1, 1, 1}] = {-1, 10};
    // topoMap[{0, 1, 1, 1}] = {14,  5};
    // topoMap[{1, 0, 1, 1}] = {12,  4};
    // topoMap[{1, 1, 0, 1}] = { 6,  1};
    // topoMap[{1, 1, 1, 0}] = { 8,  2};
    // topoMap[{0, 0, 1, 1}] = {13,  7};
    // topoMap[{1, 0, 0, 1}] = { 9, 11};
    // topoMap[{1, 1, 0, 0}] = { 7, 13};
    // topoMap[{0, 1, 1, 0}] = {11,  9};
    // topoMap[{0, 1, 0, 1}] = {16, 15};
    // topoMap[{1, 0, 1, 0}] = {15, 16};
    // topoMap[{1, 0, 0, 0}] = { 5, 14};
    // topoMap[{0, 1, 0, 0}] = { 4, 12};
    // topoMap[{0, 0, 1, 0}] = { 1,  6};
    // topoMap[{0, 0, 0, 1}] = { 2,  8};
    // topoMap[{0, 0, 0, 0}] = {10, -1};

    // Map it a different way:
    // For the current terrain type, which of the 4 nodes are my type?
    topoMap2[{0, 1, 2, 3}] = 10;
    topoMap2[{1, 2, 3}]    =  5;
    topoMap2[{0, 2, 3}]    =  4;
    topoMap2[{0, 1, 3}]    =  1;
    topoMap2[{0, 1, 2}]    =  2;
    topoMap2[{2, 3}]       =  7;
    topoMap2[{0, 3}]       = 11;
    topoMap2[{0, 1}]       = 13;
    topoMap2[{1, 2}]       =  9;
    topoMap2[{1, 3}]       = 15;
    topoMap2[{0, 2}]       = 16;
    topoMap2[{0}]          = 14;
    topoMap2[{1}]          = 12;
    topoMap2[{2}]          =  6;
    topoMap2[{3}]          =  8;
}

void TileMap::LoadTileSet(const std::string& fname)
{
    olc::Sprite sprMap(fname);

    for (int i = 0; i < N_LAYERS; i++) {
        tileSets[i] = new TileSet(_pge, &sprMap, layers[i]); 
    }
}

void TileMap::LoadTerrainMap()
{
    // Load the terrain-tile assets
    //LoadTileSet("resources/lpc-terrains/terrain-v7.png");
    LoadTileSet("resources/lpc-terrains/reduced-tileset-1.png");

    // Load the map definition file
    //std::ifstream f("test-terrain-2.dat", std::ifstream::in);
    //std::ifstream f("test-terrain-8x4.dat", std::ifstream::in);
    std::ifstream f("test.dat", std::ifstream::in);
    
    int32_t nx, ny;
    f >> nx >> ny;
    int32_t n_tiles = nx * ny;
    int32_t n_grid = (nx - 1) * (ny - 1);

    _dims.x = nx - 1;
    _dims.y = ny - 1;

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
        bMapLoaded = false;
        return;
    }

    bMapLoaded = true;
    _map.resize(n_grid);
    for (int32_t i = 0; i < n_grid; i++) {
        const int32_t ix = i % _dims.x; // x == column
        const int32_t iy = i / _dims.x; // y == row
        const int32_t tidx = (ix + 1) + (iy + 1) * nx; // Index in full world input
        const int layer = texmap[tidx];
        const TERRAIN_TYPE tt = static_cast<TERRAIN_TYPE>(layers[layer]);
        _map[i].pColor = COLORS[layer];
        _map[i].layer = layer;
        _map[i].fEffort = teffort[tt];

        //std::cout << "[" << texmap[i] << ", (" << ix << "," << iy << ")] ";
        //if ((i + 1) % 20 == 0) std::cout << std::endl;

        // Note:
        // With how we're currently creating the terrain, we need to offset
        // the sprites by half a tile size for this to actually work
        _map[i].vTileCoord = {ix, iy};
        _map[i].vScreenPos = {ix * 32.f - 16.f, iy * 32.f - 16.f};
        _map[i].pge = _pge;
    }

    // Use a neighborhood of 4 values to determine each tile's sprite
    // Note that the sprite will then be offset by 1/2 W, H in order for
    // the resultant terrain to line up with the given inputs
    for (int i = 0; i < n_grid; i++) {
        const int myL = _map[i].layer;
        std::array<int, 4> bcs = {myL, myL, myL, myL};
        const int ix = _map[i].vTileCoord.x;
        const int iy = _map[i].vTileCoord.y;
    
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
        _map[i].dTexture = new olc::Decal(tex);
        _pge->SetDrawTarget(nullptr);
        _pge->DrawDecal({0.f, 0.f}, _map[i].dTexture);
    }
};

olc::Sprite* TileMap::GetEdgeTileFor(std::array<std::vector<int>, N_LAYERS> laymap)
{
    // Returns a sprite created by layering the appropriate terrain types into
    // a single sprite, in order
  
    // For each available layer, map it's list of indices
    // to the matching tile index from its tileset
    // Default to -1 if that layer is unused
    std::array<int, N_LAYERS> tIdx;
    for (int L = 0; L < N_LAYERS; L++) {
        if (laymap[L].size() > 0) {
            tIdx[L] = topoMap2[laymap[L]];
        } else {
            tIdx[L] = -1;
        }
    }

    for (auto& t : tIdx) {
        // Add some spice - randomize all the 'plain' tiles
        if (t == TileSet::GetBaseIdx()) {
            t = TileSet::GetRandomBaseTile();
        }
    }

    olc::Sprite* spr = new olc::Sprite(32, 32);
    _pge->SetPixelMode(olc::Pixel::MASK);
    _pge->SetDrawTarget(spr);

    for (int i = 0; i < N_LAYERS; i++) {
        if (tIdx[i] >= 0 && tIdx[i] < tileSets[i]->N_TILES) {
            _pge->DrawSprite(0, 0, tileSets[i]->GetTileAt(tIdx[i]));
        }
    }

    _pge->SetDrawTarget(nullptr);

    return spr;
}

TERRAIN_TYPE TileMap::GetTerrainAt(int ix, int iy)
{
    const int idx = ix + iy * _dims.x;
    return GetTerrainAt(idx);
}

TERRAIN_TYPE TileMap::GetTerrainAt(int idx)
{
    if (idx < 0 || idx >= _map.size()) return TYPE_COUNT;

    return static_cast<TERRAIN_TYPE>(layers[_map[idx].layer]);
}

float TileMap::GetEffortAt(int ix, int iy)
{
    const int idx = ix + iy * _dims.x;
    return GetEffortAt(idx);
}

float TileMap::GetEffortAt(int idx)
{
    if (idx < 0 || idx >= _map.size()) return -1.f;

    return _map[idx].fEffort;
}

void TileMap::Draw()
{
    for (auto &T : _map) {
        T.Draw();
    }
};

