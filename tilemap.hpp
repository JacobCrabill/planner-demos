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

class Tile
{
public:
    Tile(olc::PixelGameEngine* pge, const olc::Renderable& texture, olc::vf2d pos, float weight) :
        _pge(pge), _rTexture(texture), _vPos(pos), _fWeight(weight) { };

    const olc::Renderable& GetTexture() { return _rTexture; }

    olc::Sprite* GetSprite() { return _rTexture.Sprite(); }
    olc::Decal* GetDecal() { return _rTexture.Decal(); }

    const olc::vf2d& GetPos() { return _vPos; };

    void Draw() { _pge->DrawDecal(_vPos, _rTexture.Decal()); }

private:
    const olc::Renderable& _rTexture;
    olc::vf2d _vPos;
    float _fWeight;
    olc::PixelGameEngine* _pge {nullptr};

};

class TileMap
{
public:
    TileMap() { };

    void AddTile(std::shared_ptr<Tile> tile)
    {
        _map.push_back(tile);
    }

    void Draw()
    {
        for (auto &T : _map) {
            T->Draw();
        }
    };

private:
    std::vector<std::shared_ptr<Tile> > _map;

};
