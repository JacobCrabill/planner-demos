/**
 * @File: astar.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */
#pragma once

#include "olcPixelGameEngine.h"

#include <thread>
#include <barrier>
#include <mutex>
#include <condition_variable>

#include "tilemap.hpp"

struct TileWorker
{
    // Location within the grid, and overall grid size
    olc::vi2d _loc {0, 0};
    olc::vi2d _gridDims {0, 0};

    // For the A* alg.
    float _f {FLT_MAX};
    float _g {FLT_MAX};
    bool _isStart {false};
    bool _isGoal {false};
    bool _visited {false};

    std::thread _thread;

    bool _alive {true};
    std::atomic<bool>* _goalReached;
    std::mutex _mux;
    std::condition_variable _cvWait;
    std::shared_ptr<std::barrier> _syncPoint {nullptr};

    void Reset()
    {
        _f = FLT_MAX;
        _g = FLT_MAX;
        _isStart = false;
        _isGoal = false;
        _visited = false;
        _alive = true;
    }

    // Called to start a new path solve
    void Start()
    {
        std::unique_lock<std::mutex> lock(_mux);
        _cvWait.notify_one();
    }

    void ThreadLoop()
    {
        while (alive)
        {
            // Synchronize with the parent thread setting up the problem
            std::unique_lock<std::mutex> lock(_mux);
            _cvWait.wait(lock); // Unlocks the lock while waiting for the cv

            while (!(*_goalReached)) {
                // Do one iteration
            
                // Notify that we've finished the iteration and wait to go again
                _syncPoint.arrive_and_wait();
            }
        }
    }
};

class AStar
{
public:
    AStar() { };

    void SetTerrainMap(TileMap& map);

    bool ComputePath(olc::vi2d start, olc::vi2d goal);

private:
    TileMap* _map {nullptr};

    olc::vi2d _dims {0, 0};
    std::vector<TileWorker> _workers;
    std::vector<float> _gScore;
    std::vector<float> _fScore;
    //std::priority_queue
    
    std::shared_ptr<std::barrier> _syncPoint {nullptr};
    std::atomic<bool> _goalReached {false};

    // Heuristic function (Manhattan distance, or Euclidean distance)
    float H(olc::vi2d t1, olc::vi2d t2);

};

void Astar::SetTerrainMap(TileMap& map)
{
    _map = &map;

    _dims = _map->GetDims();

    // Construct our local copy of the map in a format suitable for the algo.
    _workers.resize(_dims.x * _dims.y);

    _syncPoint = std::make_shared<std::barrier>(_workers.size());

    int n = 0;
    for (auto& w : _workers) {
        const int i = n % _dims.x;
        const int j = n / _dims.y;
        w._loc = {i, j};
        w._gridDims = _dims;
        w._syncPoint = _syncPoint;
        w._goalReached = &_goalReached;
        w._thread = std::thread(&w.ThreadLoop, &w);

        n++;
    }
}

bool Astar::ComputePath(olc::vi2d start, olc::vi2d goal)
{
    _goalReached = false;

    for (auto& w : _workers) w.reset();

    auto sLoc = start.GetPos();
    auto gLoc = goal.GetPos();
    
    int sInd = sLoc.y * _dims.x + sLoc.x;
    int gInd = gLoc.y * _dims.x + gLoc.x;

    _workers[sInd].isStart = true;
    _workers[gInd].isGoal = true;

    for (auto& w : _workers) w.start();

    while (!_goalReached) { usleep(1000); };
}
