/**
 * @File: gamemap.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a world map made of discrete Tiles
 *     Also implements the Tile class to hold a texture and terrain attributes
 */
#pragma once

#include "olcPixelGameEngine.h"
#include "util.hpp"
#include "tileset.hpp"

//! The terrain types available in my reduced tileset
enum TERRAIN_TYPE
{
  GRASS,
  WATER,
  DIRT,
  GRAVEL,
  PAVERS,
  TYPE_COUNT
};

struct MapChunk
{
    olc::vi2d coord {0, 0};
    olc::vi2d dims {16, 16};
    std::vector<Tile> tiles;
};

//! Class to load the desired map terrain, a tileset, and display the map
class GameMap
{
public:
    GameMap(const Config& _config) : config(_config) {};

    ~GameMap();

    /**
     * @brief Load the text-based map of terrain for the game
     */
    void GenerateMap();

    void SetPGE(olc::PixelGameEngine* _pge) { pge = _pge; }

    olc::vi2d GetDims() { return dims; }

    void Draw(const olc::vi2d& offset);

    TERRAIN_TYPE GetTerrainAt(int ix, int iy);
    TERRAIN_TYPE GetTerrainAt(int idx);
    float GetEffortAt(int ix, int iy);
    float GetEffortAt(int idx);

private:
    std::map<olc::vi2d, Tile> map;

    olc::vi2d dims {0, 0}; //!< Dimensions of the overall map. TODO: Use only for static maps.
    olc::vi2d idxTL {}; //!< Top-left tile coordinate on the screen
    olc::vi2d idxBR {}; //!< Btm-right tile coordinate on the screen
    bool mapLoaded {false};
    Config config;

    olc::PixelGameEngine* pge {nullptr};

    static constexpr uint8_t N_LAYERS = 5;

    /// NOTE: This could be simpler, but I'm leaving placeholders/reminders
    /// for swapping out different terrain maps in the future.
    /// Needs more thought.

    //! Our terrain layers. TODO: Shouldn't the TileSet own this?
    /// TODO: While TileSet should own the contents of the tile map file,
    /// we here should be able to remap the types onto different layers
    /// (So e.g. dirt can be layered on top of pavers, or vice-versa)    static constexpr uint8_t N_LAYERS = 5;
    const uint8_t layers[N_LAYERS] {WATER, GRASS, DIRT, GRAVEL, PAVERS};
    TileSet* tileSet {nullptr};
    std::vector<float> tRangeSums;

    const std::map<TERRAIN_TYPE, float> teffort {
        {GRASS, 3.f}, {WATER, -1.f}, {DIRT, 10.f}, {GRAVEL, 20.f}, {PAVERS, 1.f}
    };
};
