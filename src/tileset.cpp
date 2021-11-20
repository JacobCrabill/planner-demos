/**
 * @File: tileset.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Class to handle generation of terrain textures from a
 *     terrain-type texture map image
 */
#include "tileset.hpp"

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

    // Pre-generate our collection of plain and decorative base tiles
    baseTiles.resize(nTypes);

    const std::vector<int> btIds = GetDecorativeBaseTilesIndices();
    for (uint8_t L = 0; L < nTypes; L++) {
        for (int bt : btIds) {
            baseTiles[L][bt] = new olc::Decal(GetTileAt(L, bt));
        }
    }

    // Add a placeholder for a blank / emptry sprite
    blankTile.Create(TW, TH);
    pge->SetDrawTarget(blankTile.Sprite());
    pge->FillRect({0, 0}, {TW, TH}, olc::BLACK);
    pge->SetDrawTarget(nullptr);
    blankTile.UpdateDecal();
}

TileSet::~TileSet()
{
    if (tileset != nullptr)
        delete tileset;

    for (auto& tset : tiles)
        for (auto& spr : tset)
            delete spr;

    for (auto& tex : texCache) {
        delete tex.second;
    }
    texCache.clear();

    for (auto& baseL : baseTiles) {
        for (auto& bt : baseL) {
            delete bt.second;
        }
        baseL.clear();
    }
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

olc::Sprite* TileSet::GetTileAt(uint8_t type, int idx)
{
    return tiles[type][idx];
}

olc::Decal* TileSet::GetTextureFor(const std::array<uint8_t, 4>& bcs, olc::vi2d loc)
{
    if (IsBaseTile(bcs)) {
        uint8_t type = bcs[0];
        if (type > 4) {
            // Invalid type
            return blankTile.Decal();
        }
        // Use a nice, deterministic "random number" to get the "random" tile
        float rval = SimpleRand(loc.x, loc.y);
        int t = GetRandomBaseTile(rval); //GetNoise(50.*ix, 50.*iy));
        return baseTiles[type].at(t);
    }

    if (texCache.count(bcs)) {
        // Note: This should throw an error if we improperly try to get a tile that doesn't exist
        return texCache.at(bcs);
    }

    // Generate the texture, building it up as layers of sprites

    olc::Decal* dec = new olc::Decal(CreateSpriteFromBCs(bcs));
    texCache[bcs] = dec;

    return dec;
}

int TileSet::GetIdxFromTopology(const std::vector<uint8_t>& topo)
{
    // See header file for detils of how topoMap was derived
    if (topo.size() == 0 || topo.size() > 4) return -1;

    return topoMap.at(topo);
}

bool TileSet::IsBaseTile(const std::array<uint8_t, 4>& bcs)
{
    const int bc0 = bcs[0];
    for (uint8_t i = 1; i < 4; i++) {
        if (bcs[i] != bc0) {
            return false;
        }
    }

    return true;
}

olc::Sprite* TileSet::CreateSpriteFromBCs(const std::array<uint8_t, 4>& bcs)
{
    // Returns a sprite created by layering the appropriate terrain types into
    // a single sprite, in order
    PROFILE_FUNC();

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

    // For each available layer, map its list of indices
    // to the matching tile index from its tileset
    // Default to -1 if that layer is unused
    std::array<int, N_LAYERS> tIdx;
    for (uint8_t L = 0; L < N_LAYERS; L++) {
        if (laymap[L].size() > 0) {
            tIdx[L] = GetIdxFromTopology(laymap[L]);
        } else {
            tIdx[L] = -1;
        }
    }

    for (auto& t : tIdx) {
        // Add some spice - randomize all the 'plain' tiles
        /// TODO: This shouldn't dynamically adding/removing tiles make this weird(?)
        if (t == GetBaseIdx()) {
            t = GetRandomBaseTile();
        }
    }

    olc::Sprite* spr = new olc::Sprite(TW, TH);
    pge->SetPixelMode(olc::Pixel::MASK);
    pge->SetDrawTarget(spr);

    for (uint8_t L = 0; L < N_LAYERS; L++) {
        if (tIdx[L] >= 0 && tIdx[L] < TS_N_TILES) {
            pge->DrawSprite(0, 0, GetTileAt(L, tIdx[L]));
        }
    }

    pge->SetDrawTarget(nullptr);

    return spr;
}
