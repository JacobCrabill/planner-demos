/**
 * @File: planner.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     ABC to define the motion-planner API
 */
#pragma once

#include "olcPixelGameEngine.h"

#include <vector>

#include "gamemap.hpp"

class Planner
{
public:
    virtual void SetTerrainMap(GameMap& map) = 0;

    virtual bool ComputePath(olc::vi2d start, olc::vi2d goal) = 0;

    virtual std::vector<olc::vi2d> GetPath() = 0;
    
    virtual float GetPathCost() = 0;
};

