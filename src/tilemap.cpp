/**
 * @File: tilemap.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#include "tilemap.hpp"

#include <unistd.h> /// DEBUGGING

static std::array<olc::Pixel, 18> COLORS {
    olc::VERY_DARK_GREY, olc::VERY_DARK_RED, olc::VERY_DARK_YELLOW,
        olc::VERY_DARK_CYAN, olc::VERY_DARK_BLUE,
    olc::DARK_GREY, olc::DARK_RED, olc::DARK_YELLOW, olc::DARK_CYAN, olc::DARK_BLUE,
    olc::GREY, olc::RED, olc::YELLOW, olc::CYAN, olc::BLUE,
    olc::WHITE, olc::BLACK, olc::BLANK
};

// struct tileset_v7
// {
//     // Laying out the structure of lpc-terrains-v7.png
//     const int tile_w = 32;
//     const int tile_h = 32;
//     const int nx_per_type = 3;
//     const int ny_per_type = 7;
//     const int ntypes_x = 10;
//     const int ntypes_y = 4;
// }

void Tile::Draw()
{
    if (pge) {
        // Use the supplied texture if we have it
        if (dTexture) {
            pge->DrawDecal(vScreenPos, dTexture); //->Decal());
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
}

void TileMap::LoadTileSet(const std::string& fname)
{
    //rAll.Load(fname);
    //tileset.resize(128);
    //int n = 0;
    //for (auto &rTile : tileset) {
    //    rTile.Create(32, 32);
    //    _pge->SetDrawTarget(rTile.Sprite());
    //    const int ox = (n % 16) * 32;
    //    const int oy = (n / 16) * 32;
    //    _pge->DrawPartialSprite(0, 0, rAll.Sprite(), ox, oy, 32, 32);
    //    // Note: Drawing to the sprite of a renderable does not
    //    // automatically update the decal
    //    rTile.UpdateDecal();
    //    n++;
    //}
    //_pge->SetDrawTarget(nullptr);

    olc::Sprite sprMap(fname);

    for (int i = 0; i < 3; i++) {
        tiles[i] = new TileSet(_pge, &sprMap, layers[i]); 
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
    int n = 0;
    for (auto& i : texmap) {
        i = std::min(std::max(0, i), n_layers);
        n++;
        printf("%d ", i);
        if (n % nx == 0) printf("\n");
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
        _map[i].pColor = COLORS[texmap[i]];
        _map[i].layer = texmap[i];
        _map[i].fEffort = float(texmap[i]); /// TODO: Map terrain types to effort
        const int32_t ix = i % _dims.x; // x == column
        const int32_t iy = i / _dims.x; // y == row

        //std::cout << "[" << texmap[i] << ", (" << ix << "," << iy << ")] ";
        //if ((i + 1) % 20 == 0) std::cout << std::endl;

        _map[i].vTileCoord = {ix, iy};
        _map[i].vScreenPos = {ix * 32.f, iy * 32.f};
        _map[i].pge = _pge;
    }

    // Correct Boundaries (inter-type edges)
    for (int i = 0; i < n_grid; i++) {
        const int myL = _map[i].layer;
        std::array<int, 4> bcs = {myL, myL, myL, myL};
        const int ix = _map[i].vTileCoord.x;
        const int iy = _map[i].vTileCoord.y;
    
        if (ix < nx && iy < ny) {
            bcs[0] = texmap[ix + (iy * nx)];
            bcs[1] = texmap[ix + 1 + (iy * nx)];
            bcs[2] = texmap[ix + 1 + (iy + 1) * nx];
            bcs[3] = texmap[ix + (iy + 1) * nx];
        }

        // OLD METHOD -- TOO HARD TO WORK WITH THIS TILESET
        // Neighbors: Top, Right, Bottom, Left
        // if (iy > 0)           bcs[0] = _map[i - _dims.x].layer;
        // if (ix + 1 < _dims.x) bcs[1] = _map[i + 1].layer;
        // if (iy + 1 < _dims.y) bcs[2] = _map[i + _dims.x].layer;
        // if (ix > 0)           bcs[3] = _map[i - 1].layer;

        olc::Sprite* tex = GetEdgeTileFor(myL, bcs);
        _map[i].dTexture = new olc::Decal(tex);
        _pge->SetDrawTarget(nullptr);
        _pge->DrawDecal({0.f, 0.f}, _map[i].dTexture);
        usleep(100);
    }
};

olc::Sprite* TileMap::GetEdgeTileFor(int myL, std::array<int, 4> bcs)
{
    // Returns a sprite created by layering the appropriate terrain types into
    // a single sprite, in order
    int min_layer = TERRAIN_TYPE::TYPE_COUNT;

    min_layer = std::min(min_layer, myL);
    for (int i = 0; i < 4; i++) {
        min_layer = std::min(min_layer, bcs[i]);
    }

    /// TODO: Relocate / refactor
    // Map each possible boundary condition to an overlay tile
    /**
     * We are using an offset grid to allow finer control of the terrain
     * stitching with our given tileset
     * e.g. - The input map of tile terrain values is offset by 1/2 of a tile
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

    std::map<std::array<int, 4>, std::array<int, 2>> tmap;
    tmap[{1, 1, 1, 1}] = {10, -1};
    tmap[{0, 1, 1, 1}] = {5, 14};
    tmap[{1, 0, 1, 1}] = {4, 12};
    tmap[{1, 1, 0, 1}] = {1, 6};
    tmap[{1, 1, 1, 0}] = {2, 8};
    tmap[{0, 0, 1, 1}] = {7, 13};
    tmap[{1, 0, 0, 1}] = {11, 9};
    tmap[{1, 1, 0, 0}] = {13, 7};
    tmap[{0, 1, 1, 0}] = {9, 11};
    tmap[{0, 1, 0, 1}] = {15, 16};
    tmap[{1, 0, 1, 0}] = {16, 15};
    tmap[{1, 0, 0, 0}] = {14, 5};
    tmap[{0, 1, 0, 0}] = {12, 4};
    tmap[{0, 0, 1, 0}] = {6, 1};
    tmap[{0, 0, 0, 1}] = {8, 2};
    tmap[{0, 0, 0, 0}] = {-1, 10};

    auto tIdx = tmap[bcs];

    olc::Sprite* spr = new olc::Sprite(32, 32);
    _pge->SetPixelMode(olc::Pixel::MASK);
    _pge->SetDrawTarget(spr);

    /// TODO: Fix layer numbering
    if (tIdx[1] >= 0 && tIdx[1] < 21) {
        _pge->DrawSprite(0, 0, tiles[0]->GetTileAt(tIdx[1]));
    }

    if (tIdx[0] >= 0 && tIdx[0] < 21) {
        _pge->DrawSprite(0, 0, tiles[1]->GetTileAt(tIdx[0]));
    }
    _pge->SetDrawTarget(nullptr);

    return spr;
}

void TileMap::Draw()
{
    for (auto &T : _map) {
        T.Draw();
    }
    //int n = 0;
    //for (auto &rTile : tileset) {
    //    const int ox = (n % 20) * 32;
    //    const int oy = (n / 20) * 32;
    //    _pge->DrawDecal(olc::vi2d({ox,oy}), rTile.Decal());
    //    n++;
    //}

    // DEBUGGING - draw the individual terrain tilesets
    // tiles[0]->DrawSingleTile({0, 128}, {0, 0});
    // tiles[1]->DrawSingleTile({32*3, 128}, {0, 0});
    // tiles[2]->DrawSingleTile({32*6, 128}, {0, 0});
};

