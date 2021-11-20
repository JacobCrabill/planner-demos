/**
 * @File: tileset.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements a texture atlas interface for the rest of the game
 */
#pragma once

#include "olcPixelGameEngine.h"
#include "util.hpp"

#include <cassert>
#include <random>

#ifndef TW
#define TW 32
#endif
#ifndef TH
#define TH 32
#endif

//! A single tile in our game map.
struct Tile
{
    void Draw(const olc::vi2d& ofset);

    olc::PixelGameEngine* pge {nullptr};

    olc::Decal* dTexture {nullptr}; //!< The texture to display
    olc::vi2d vTileCoord;           //!< The (i,j) coordinates of this tile within the game map
    olc::vf2d vScreenPos;           //!< The location _of the sprite_ in the screen frame
    float fEffort {0.f};            //!< The effort required to cross this tile
    uint8_t layer {0};              //!< Which terrain-style layer this tile is
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
     * @param fname Sprite file containing an array of tilesets for all terrain types
     * @param typeMap Remapping of game layers to terrain types from the tileset
     */
    TileSet(olc::PixelGameEngine* pge, std::string fname, const uint8_t* typeMap, uint8_t nTypes);

    ~TileSet();

    olc::Sprite* GetBaseTile(uint8_t type);
    olc::Sprite* GetTileAt(uint8_t type, int idx);
    olc::Decal* GetTextureFor(const std::array<uint8_t, 4>& bcs, olc::vi2d loc);

    uint8_t GetNTypes() { return (uint8_t)tiles.size(); }

private:

    /**
     * Get a random 'plain terrain' tile, allowing us to add some variety to
     * otherwise boring regions.
     *
     * @param rval: Optional random number to use in range [0, 1]
     */
    int GetRandomBaseTile(const float rval = -1.f);

    std::vector<int> GetDecorativeBaseTilesIndices() { return std::vector<int> {10, 18, 19, 20}; };

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
    int GetIdxFromTopology(const std::vector<uint8_t>& topo);

    //! The index of a 'plain' tile of this terrain type
    int GetBaseIdx() { return 10; }

    //! Width and height (in px) of each terrain type in the overall tileset
    const int TS_NX = 3;            //!< Number of tiles in X direction
    const int TS_NY = 7;            //!< Number of tiles in Y direction
    const int TS_W = TS_NX * TW;      //!< Width of the tileset (pixels)
    const int TS_H = TS_NY * TH;      //!< Height of the tileset (pixels)
    const int TS_N_TILES = TS_NX * TS_NY; //!< Number of individual tiles in the tileset

    static constexpr uint8_t N_LAYERS = 5;

    olc::PixelGameEngine* pge {nullptr};

    //! Map from the topology of the terrain input to a terrain tile index.
    const std::map<std::vector<uint8_t>, int> topoMap;

    olc::Sprite* tileset {nullptr};   //!< The entire tileset for all terrain types
    std::vector<std::vector<olc::Sprite*>> tiles;  //!< Each individual tile for all terrain types
    olc::Renderable blankTile;

    std::map<std::array<uint8_t, 4>, olc::Decal*> texCache; //!< Cache of all previously-generated tile textures
    std::vector<std::map<int, olc::Decal*>> baseTiles; // One for each terrain type

    bool IsBaseTile(const std::array<uint8_t, 4>& bcs);
    bool CacheHit(const std::array<uint8_t, 4>& bcs);

    /**
     * @brief Create a layered sprite for the topology of our 2x2 region
     */
    olc::Sprite* CreateSpriteFromBCs(const std::array<uint8_t, 4>& bcs);

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
