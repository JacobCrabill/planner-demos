/**
 * @File: tilemap.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#pragma once

#include "olcPixelGameEngine.h"

#ifndef W
#define W 32
#endif
#ifndef H
#define H 32
#endif

struct Tile
{
    olc::PixelGameEngine* pge {nullptr};
    olc::Decal* dTexture {nullptr};
    float fEffort {0.f};
    olc::vi2d vTileCoord;
    olc::vf2d vScreenPos;
    int layer {0};
    olc::Pixel pColor {olc::BLANK};
    
    // int32_t gridW {32}; // Width of each column in our tile map
    // int32_t gridH {32}; // Height of each row in our tile map
    // int32_t tileW {32}; // Width of _this_ tile
    // int32_t tileH {32}; // Height of _this_ tile

    void Draw();
};

// Struct to hold and use the tileset for one terrain type
class TileSet
{
public:
    TileSet(olc::PixelGameEngine* pge, olc::Sprite* sprMap, int tIdx);
    ~TileSet();

    olc::Sprite* tileset {nullptr};
    std::vector<olc::Sprite*> tiles;

    void DrawSingleTile(const olc::vi2d sLoc, const olc::vi2d tIdx);
    void DrawBaseTile(const olc::vi2d sLoc);
    olc::Sprite* GetBaseTile();
    olc::Sprite* GetTileAt(int idx);

    int GetBaseIdx() { return 10; }
    int GetOTLIdx() { return 6; }  // Overlay | Top-Left
    int GetOTCIdx() { return 7; }  // Overlay | Top-Center
    int GetOTRIdx() { return 8; }  // Overlay | Top-Right
    int GetOLCIdx() { return 9; }  // Overlay | Left-Center
    int GetORCIdx() { return 11; } // Overlay | Right-Center
    int GetOBLIdx() { return 12; } // Overlay | Bottom-Left
    int GetOBCIdx() { return 13; } // Overlay | Bottom-Center
    int GetOBRIdx() { return 14; } // Overlay | Bottom-Right
    int GetSingletIdx() { return 3 * (rand() % 2); } // Single-tile overlay

    // Width and height (in px) of each terrain type in the map
    const int TW = 3 * 32;
    const int TH = 7 * 32;

private:
    olc::PixelGameEngine* _pge {nullptr};
};

/** My reduced-selection tileset setttings:
 *
 * - 5 Terrain types [Grass, Water, Dirt, Gravel, Pavers]
 * - Tile size: 32 x 32 pixels
 * - Terrain-type tileset size: 3 x 7 tiles each [W x H]
 * - Layout:
 *   - Single-Tile Decorative Overlays: [[0, 0], [1, 0]]
 *   - Inset-terrain corners: [[0, 1], [0, 2]; [1, 1], [1, 2]]
 *   - Edging and baseline: [[2, 0] through [4, 2]]
 *   - Diagonals: [[5, 0], [5, 1]]
 *   - Base-Tile Variations: [[5, 2]; [6, 0], [6, 1], [6, 2]]
 */
enum TERRAIN_TYPE
{
  GRASS,
  WATER,
  DIRT,
  GRAVEL,
  PAVERS,
  TYPE_COUNT
};

class TileMap
{
public:
    TileMap();

    void LoadTileSet(const std::string& fname);

    void LoadTerrainMap();

    void SetPGE(olc::PixelGameEngine* pge) { _pge = pge; }

    olc::vi2d GetDims() { return _dims; }

    void Draw();

private:
    std::vector<Tile> _map;

    olc::vi2d _dims {0, 0};
    bool bMapLoaded {false};

    olc::PixelGameEngine* _pge {nullptr};

    // Our terrain layers
    static constexpr int n_layers = 2;
    const int layers[n_layers] {WATER, GRASS};
    TileSet* tileSets[n_layers] {nullptr};

    // Map from the topology of the terrain input to a terrain tile
    std::map<std::array<int, 4>, std::array<int, 2>> topoMap;

    // Create a layered sprite for the given layer + neighboring layers
    olc::Sprite* GetEdgeTileFor(int myL, std::array<int, 4> bcs);

};
