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

#include <cassert>
#include <random>

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

    // Allow us to add some variety to the plain regions
    static int GetRandomBaseTile()
    {
        int baseTileIds[4] = {10, 18, 19, 20};
        int baseTileWgts[4] = {15, 1, 1, 1};

        int wSum = 0;
        for (int i = 0; i < 4; i++) {
            wSum += baseTileWgts[i];
        }

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

    static int GetBaseIdx() { return 10; }

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
    const int N_TILES = TW * TH;

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

    TERRAIN_TYPE GetTerrainAt(int ix, int iy);
    TERRAIN_TYPE GetTerrainAt(int idx);
    float GetEffortAt(int ix, int iy);
    float GetEffortAt(int idx);

private:
    std::vector<Tile> _map;

    olc::vi2d _dims {0, 0};
    bool bMapLoaded {false};

    olc::PixelGameEngine* _pge {nullptr};

    // Our terrain layers
    static constexpr int N_LAYERS = 4;
    const int layers[N_LAYERS] {WATER, DIRT, GRASS, PAVERS};
    TileSet* tileSets[N_LAYERS] {nullptr};

    // Map from the topology of the terrain input to a terrain tile
    std::map<std::array<int, 4>, std::array<int, 2>> topoMap;
    std::map<std::vector<int>, int> topoMap2;
    std::map<TERRAIN_TYPE, float> teffort {
        {GRASS, 3.f}, {WATER, -1.f}, {DIRT, 10.f}, {GRAVEL, 20.f}, {PAVERS, 1.f}
    };

    // Create a layered sprite for the given the topology of our 2x2 region
    olc::Sprite* GetEdgeTileFor(std::array<std::vector<int>, N_LAYERS> laymap);
};
