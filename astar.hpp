/**
 * @File: astar.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */
#pragma once

#include "olcPixelGameEngine.h"

#include "tilemap.hpp"

class AStar
{
public:
    AStar(std::shared_ptr<TileMap> map) :
        _map(map) { };

private:
    std::shared_ptr<TileMap> _map;

};
