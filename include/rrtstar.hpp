/**
 * @File: rrtstar.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the RRT* algorithm for a 2D array of tiles
 */
#pragma once

#include "olcPixelGameEngine.h"

#include <cfloat>
#include <queue>

#include "gamemap.hpp"
#include "planner.hpp"


class RRTStar : public Planner
{
public:
    RRTStar() { };

    void SetTerrainMap(GameMap& map) override;

    bool ComputePath(olc::vi2d start, olc::vi2d goal) override;

    std::vector<olc::vi2d> GetPath() override;
    float GetPathCost() override { return path_cost; }

private:
    GameMap* map {nullptr};

    olc::vi2d dims {0, 0};
    
    bool goalReached {false};
    float path_cost {-1.f};

    std::vector<olc::vi2d> final_path;
};

