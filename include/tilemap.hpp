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

static std::array<olc::Pixel, 18> COLORS {
    olc::VERY_DARK_GREY, olc::VERY_DARK_RED, olc::VERY_DARK_YELLOW,
        olc::VERY_DARK_CYAN, olc::VERY_DARK_BLUE,
    olc::DARK_GREY, olc::DARK_RED, olc::DARK_YELLOW, olc::DARK_CYAN, olc::DARK_BLUE,
    olc::GREY, olc::RED, olc::YELLOW, olc::CYAN, olc::BLUE,
    olc::WHITE, olc::BLACK, olc::BLANK
};

struct Tile
{
    olc::PixelGameEngine* pge {nullptr};
    std::shared_ptr<olc::Renderable> rTexture {nullptr};
    float fEffort {0.f};
    olc::vi2d vTileCoord;
    olc::vf2d vScreenPos;
    olc::Pixel pColor {olc::BLANK};
    
    // int32_t gridW {32}; // Width of each column in our tile map
    // int32_t gridH {32}; // Height of each row in our tile map
    // int32_t tileW {32}; // Width of _this_ tile
    // int32_t tileH {32}; // Height of _this_ tile

    void Draw()
    {
        if (pge) {
            // Use the supplied texture if we have it
            if (rTexture) {
                pge->DrawDecal(vScreenPos, rTexture->Decal());
            } else {
                // Otherwise draw a simple filled rectangle
                const int32_t x = vTileCoord.x * W;
                const int32_t y = vTileCoord.y * H;
                pge->FillRect(x, y, W, H, pColor);
            }
        }
    }
};


class TileMap
{
public:
    TileMap() { };

    void LoadTileSet(const std::string& fname)
    {
        rAll.Load(fname);
        tileset.resize(128);
        int n = 0;
        for (auto &rTile : tileset) {
            rTile.Create(32, 32);
            _pge->SetDrawTarget(rTile.Sprite());
            const int ox = (n % 16) * 32;
            const int oy = (n / 16) * 32;
            _pge->DrawPartialSprite(0, 0, rAll.Sprite(), ox, oy, 32, 32);
            // Note: Drawing to the sprite of a renderable does not
            // automatically update the decal
            rTile.UpdateDecal();
            n++;
        }
        _pge->SetDrawTarget(nullptr);

    }

    void LoadTerrainMap()
    {
        LoadTileSet("resources/lpc-terrains/terrain-v7.png");

        std::ifstream f("test.dat", std::ifstream::in);
        
        int32_t nx, ny;
        f >> nx >> ny;
        int32_t n_tiles = nx * ny;

        _dims.x = nx;
        _dims.y = ny;

        std::vector<int32_t> texmap(n_tiles);
        int32_t n_read = 0;
        while (f >> texmap[n_read]) n_read++;

        if (n_read < n_tiles) {
            std::cout << "Error while reading texture map - unexpected EOF";
            std::cout << std::endl;
            bMapLoaded = false;
            return;
        }

        bMapLoaded = true;
        _map.resize(n_read);
        for (int32_t i = 0; i < n_tiles; i++) {
            _map[i].pColor = COLORS[texmap[i]];
            _map[i].fEffort = static_cast<float>(texmap[i]);
            const int32_t ix = i % nx; // x == column
            const int32_t iy = i / ny; // y == row
            _map[i].vTileCoord = {ix, iy};
            _map[i].vScreenPos = {ix * 32.f, iy * 32.f};
            _map[i].pge = _pge;
        }
    };

    void Draw()
    {
        for (auto &T : _map) {
            //T.Draw();
        }
        int n = 0;
        for (auto &rTile : tileset) {
            const int ox = (n % 20) * 32;
            const int oy = (n / 20) * 32;
            _pge->DrawDecal(olc::vi2d({ox,oy}), rTile.Decal());
            n++;
        }
    };

    void SetPGE(olc::PixelGameEngine* pge) { _pge = pge; }

    olc::vi2d GetDims() { return _dims; }

private:
    std::vector<Tile> _map;

    olc::vi2d _dims {0, 0};
    bool bMapLoaded {false};

    olc::PixelGameEngine* _pge {nullptr};

    olc::Renderable rAll;
    olc::Renderable rOne;
    std::vector<olc::Renderable> tileset;
};
