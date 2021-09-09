/**
 * @File: threaded_astar.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */
#pragma once

#include "olcPixelGameEngine.h"

#include <cfloat>
#include <queue>

#include "tilemap.hpp"

struct Node
{
    Node() = default;

    olc::vi2d loc {0, 0};
    int idx {0};

    float f {FLT_MAX};
    float g {FLT_MAX};
    float h {FLT_MAX};
    float effort {0.f};
    int counter {0};

    std::vector<int> neighbors;

    enum State {CLOSED, OPEN, VISITED};
    State state {CLOSED};

    std::tuple<float, int, int> GetTuple() {
        return std::make_tuple(f, counter, idx);
    }

    bool operator<(const Node& b) {
        if (f == b.f)
            return counter < b.counter;
        return f < b.f;
    }
};

class AStar
{
public:
    AStar() { };

    void SetTerrainMap(GameMap& map);

    bool ComputePath(olc::vi2d start, olc::vi2d goal);

    std::vector<olc::vi2d> GetPath();
    float GetPathCost() { return path_cost; }

private:
    GameMap* map {nullptr};

    olc::vi2d dims {0, 0};
    std::vector<Node> nodes; /// TODO: Use one of my ndarray/matrix headers
    
    bool goalReached {false};
    float path_cost {-1.f};

    std::vector<olc::vi2d> final_path;

    // Heuristic function (Manhattan distance, or Euclidean distance)
    float Hval(olc::vi2d t1, olc::vi2d t2);
};

