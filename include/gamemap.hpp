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

#include <cassert>
#include <random>

#ifndef W
#define W 32
#endif
#ifndef H
#define H 32
#endif

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

//! A single tile in our game map.
struct Tile
{
    void Draw();

    olc::PixelGameEngine* pge {nullptr};

    olc::Decal* dTexture {nullptr}; //!< The texture to display
    olc::vi2d vTileCoord;           //!< The (i,j) coordinates of this tile within the game map
    olc::vf2d vScreenPos;           //!< The location _of the sprite_ in the screen frame
    float fEffort {0.f};            //!< The effort required to cross this tile
    int layer {0};                  //!< Which terrain-style layer this tile is    
};

/**
 * @brief Struct to hold and use the sprite tileset for several terrain types.
 *
 * A very specific structure is assumed for the tileset layout and topology.
 */
class TileSet
{
public:
    /**
     * @param pge Pointer to the parent PixelGameEngine
     * @param fname Sprinte file containing an array of tilesets for all terrain types
     * @param typeMap Remapping of game layers to terrain types from the tileset
     */
    TileSet(olc::PixelGameEngine* pge, std::string fname, const int* typeMap, uint8_t nTypes);

    ~TileSet();

    olc::Sprite* tileset {nullptr};   //!< The entire tileset for all terrain types
    std::vector<std::vector<olc::Sprite*>> tiles;  //!< Each individual tile for all terrain types

    olc::Sprite* GetBaseTile(uint8_t type);
    olc::Sprite* GetTileAt(uint8_t type, int idx);

    /** 
     * Get a random 'plain terrain' tile, allowing us to add some variety to
     * otherwise boring regions.
     */
    int GetRandomBaseTile();

    /**
     * @brief Get the required tile ID for the input topology.
     * 
     * @param topo The indices of the 2x2 topo region for one terrain type. The
     *             indices must be sorted.  The ordering is (top-left,
     *             top-right, bottom-right, bottom-left).  Ex: {0,2,3}.
     * 
     * We are using an offset grid to allow finer control of the terrain
     * stitching _with our given tileset_
     *
     * e.g.: The displayed sprite is offset by 1/2 of a tile width/height from
     * the input map of tile terrain values (the game map) such that each
     * displayed tile determines its final value from the 'corner' between 4
     * input values.
     * 
     * For reference, all of the possible combinations of the 4-cell region
     * used to build the terrain sprite from are listed below, from the
     * perspective of a terrain type 'X' and some other type(s) 'O'.
     * 
     * The way we are actually building this sprite, however, is the following:
     * 1. Start with the just the first terrain layer.
     * 2. Specify the topology as seen by only this layer.
     * 3. Grab the corresponding tile for just that topology.
     * 4. Apply this tile to our sprite canvas.
     * 5. Repeat for each additional layer.
     *
     *     X X => Topology for X: {0, 1, 2, 3}
     *     X X    Tile Index: Base Tile (10, or decorative equivalent)
     *
     *     O X => Topology for X: {1, 2, 3}
     *     X X    Tile Index: Top-Left Cutout (5)
     * 
     *     X O => Topology for X: {0, 2, 3}
     *     X X    Tile Index: Top-Right Cutout (4)
     *   
     *     X X => Topology for X: {0, 1, 3}
     *     X O    Tile Index: Bottom-Right Cutout (1)
     *
     *     X X => Topology for X: {0, 1, 2}
     *     O X    Tile Index: Bottom-Left Cutout (2)
     *
     *     O O => Topology for X: {2, 3}
     *     X X    Tile Index: Top Center Overlay (7)
     *
     *     X O => Topology for X: {0, 3}
     *     X O    Tile Index: Right Center Overlay (11)
     *
     *     X X => Topology for X: {0, 1}
     *     O O    Tile Index: Bottom Center Overlay (13)
     *
     *     O X => Topology for X: {1, 2}
     *     O X    Tile Index: Left Center Overlay (9)
     *
     *     O X => Topology for X: {1, 3}
     *     X O    Tile Index: BL/TR Diag (15)
     *
     *     X O => Topology for X: {0, 2}
     *     O X    Tile Index: TL/BR Diat (16)
     *
     *     X O => Topology for X: {0}
     *     O O    Tile Index: Bottom-Right Overlay (14)
     *
     *     O X => Topology for X: {1}
     *     O O    Tile Index: Bottom-Left Overlay (12)
     *
     *     O O => Topology for X: {2}
     *     O X    Tile Index: Top-Left Overlay (6)
     *
     *     O O => Topology for X: {3}
     *     X O    Tile Index: Top-Right Overlay (8)
     *
     *     O O => Topology for X: {}
     *     O O    Tile Index: N/A
     */
    int GetIdxFromTopology(const std::vector<int>& topo);

    //! The index of a 'plain' tile of this terrain type
    int GetBaseIdx() { return 10; }

    //! Width and height (in px) of each terrain type in the overall tileset
    const int NX = 3;            //!< Number of tiles in X direction
    const int NY = 7;            //!< Number of tiles in Y direction
    const int TW = NX * 32;      //!< Width of the tileset (pixels)
    const int TH = NY * 32;      //!< Height of the tileset (pixels)
    const int N_TILES = NX * NY; //!< Number of individual tiles in the tileset

private:
    olc::PixelGameEngine* pge {nullptr};

    //! Map from the topology of the terrain input to a terrain tile index.
    const std::map<std::vector<int>, int> topoMap;

    int GetOTLIdx() { return 6; }  //!< ID for Overlay | Top-Left
    int GetOTCIdx() { return 7; }  //!< ID for Overlay | Top-Center
    int GetOTRIdx() { return 8; }  //!< ID for Overlay | Top-Right
    int GetOLCIdx() { return 9; }  //!< ID for Overlay | Left-Center
    int GetORCIdx() { return 11; } //!< ID for Overlay | Right-Center
    int GetOBLIdx() { return 12; } //!< ID for Overlay | Bottom-Left
    int GetOBCIdx() { return 13; } //!< ID for Overlay | Bottom-Center
    int GetOBRIdx() { return 14; } //!< ID for Overlay | Bottom-Right

    int GetSingletIdx() { return 3 * (rand() % 2); } //!< ID(s) for Single-tile overlay
};

//! Class to load the desired map terrain, a tileset, and display the map
class GameMap
{
public:
    GameMap(const Config& _config) : config(_config) {};

    /**
     * @brief Load the text-based map of terrain for the game
     * 
     * TODO: The input file format is too simplistic. Make it better / use YAML.
     */
    void GenerateMap();

    void SetPGE(olc::PixelGameEngine* _pge) { pge = _pge; }

    olc::vi2d GetDims() { return dims; }

    void Draw();

    TERRAIN_TYPE GetTerrainAt(int ix, int iy);
    TERRAIN_TYPE GetTerrainAt(int idx);
    float GetEffortAt(int ix, int iy);
    float GetEffortAt(int idx);

private:
    std::vector<Tile> map;

    olc::vi2d dims {0, 0};
    bool mapLoaded {false};
    Config config;

    olc::PixelGameEngine* pge {nullptr};

    //! Our terrain layers
    static constexpr int N_LAYERS = 5;
    const int layers[N_LAYERS] {WATER, GRASS, DIRT, GRAVEL, PAVERS};
    TileSet* tileSet {nullptr};

    const std::map<TERRAIN_TYPE, float> teffort {
        {GRASS, 3.f}, {WATER, -1.f}, {DIRT, 10.f}, {GRAVEL, 20.f}, {PAVERS, 1.f}
    };

    /**
     * @brief Create a layered sprite for the topology of our 2x2 region
     * 
     * @param laymap The (btm-to-top) list of terrain types for each layer
     */
    olc::Sprite* GetEdgeTileFor(std::array<std::vector<int>, N_LAYERS> laymap);
};
