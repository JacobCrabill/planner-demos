/**
 * @File: threaded_astar.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */
#pragma once

#include "olcPixelGameEngine.h"

#include <thread>
#include <barrier> // Note: Need gcc >= 11
#include <mutex>
#include <condition_variable>
#include <cfloat>

#include "tilemap.hpp"

/* The std::barrier class is templated based on a completion functor
 * Since I don't need one right now, here's the simplest definition of one
 * that does nothing
 */
struct MyEmptyCompletionF
{
    void operator()() noexcept {};
};
// To save typing and improve readability, *this* is our std::barrier
typedef std::barrier<MyEmptyCompletionF> MyBarrier;

class TileWorker
{
public:
    TileWorker() = default;

    // Needed to create a std::vector<TileWorker> using resize()
    // instead of emplace_back()
    TileWorker(const TileWorker& newTile) { /* void */ };

    // Location within the grid, and overall grid size
    olc::vi2d _loc {0, 0};
    olc::vi2d _gridDims {0, 0};
    olc::vi2d _goalLoc {0, 0};

    // For the A* alg.
    float _f {FLT_MAX};
    float _g {FLT_MAX};
    float _h {FLT_MAX};
    float _effort {0.f};
    bool _isStart {false};
    bool _isGoal {false};
    bool _visited {false};

    enum Neighbor {TOP, BOTTOM, LEFT, RIGHT};
    enum State {CLOSED, OPEN, VISITED};
    State _status {CLOSED};

    /// HACK
    TileWorker* _tileptr {nullptr};

    // Multithreading setup and synchronization
    std::thread _thread;

    bool _alive {true};
    std::atomic<bool>* _goalReached {nullptr};
    std::mutex _mux;
    std::condition_variable _cvWait;
    std::shared_ptr<MyBarrier> _syncPoint {nullptr};

    // Clears all variables back to initial state
    void Reset();

    // Called to start a new path solve
    void Start();

    // The thread will live inside this (semi-)infinite loop
    // The condition variable will be used to kick off the next run of the algorithm
    void ThreadLoop();

    // Get a neighboring tile from the tile map
    TileWorker* GetNeighbor(Neighbor n);

    float HVal();
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
    
    std::shared_ptr<MyBarrier> _syncPoint {nullptr};

    std::atomic<bool> _goalReached {false};

    // Heuristic function (Manhattan distance, or Euclidean distance)
    float Hval(olc::vi2d t1, olc::vi2d t2) { return 0.f; };

};

