/**
 * @File: threaded_astar.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */

#include "astar.hpp"

#include <cassert>

void TileWorker::Reset()
{
    std::unique_lock<std::mutex> lock(_mux);
    _f = FLT_MAX;
    _g = FLT_MAX;
    _isStart = false;
    _isGoal = false;
    _visited = false;
    _alive = true;
};

// Called to start a new path solve
void TileWorker::Start()
{
    std::unique_lock<std::mutex> lock(_mux);
    _cvWait.notify_one();
};

// The thread will live inside this (semi-)infinite loop
// The condition variable will be used to kick off the next run of the algorithm
void TileWorker::ThreadLoop()
{
    assert(_goalReached != nullptr);

    while (_alive)
    {
        // Synchronize with the parent thread setting up the problem
        std::unique_lock<std::mutex> lock(_mux);
        _cvWait.wait(lock); // Unlocks the lock while waiting for the cv
        std::cout << "T(" << _loc.x << "," << _loc.y << ") enter" << std::endl;

        // Initialize the alg.
        /// TODO
        // Set the initial heuristic (distance) value
        // Set the start tile to OPEN
        if (_isStart) {
            _status = State::OPEN;
        }

        _h = HVal();

        while (!(*_goalReached)) {
            // Do one iteration
            // - If we are the start node:
            //   - If we have not yet been 'visited':
            //     - Set ourselves to VISITED
            //     - Set all neighbors to OPEN
            // - If we are *NOT* in the OPEN set:
            //   - <skip to end>
            // - Check all neighbors:
            //   - If any neighbor is in the VISITED set:
            //     - Add its distance val to ours
            //     - If current combined val is the smallest of all neighbors,
            //       update the stored distance value and set our ancestor to 
            //       the current neighbor
            //   - If any neighbor is in the CLOSED set:
            //     - Set the neighbor to OPEN

            if (_status == OPEN) {
                // Check Top neighbor
                Tile* n = GetNeighbor(Neighbor::TOP);
                // Check Bottom neighbor
                // Check Left neighbor
                // Check Right neighbor
            }
        
            // Notify that we've finished the iteration and wait to go again
            _syncPoint->arrive_and_wait();

            // TODO
            if (_isGoal) {
                //std::cout << "T(" << _loc.x << "," << _loc.y << ") goalReached" << std::endl;
                *_goalReached = true;
            }

            _syncPoint->arrive_and_wait();
            //std::cout << "T(" << _loc.x << "," << _loc.y << ") val: " << *_goalReached << std::endl;
        }
    }
};

float TileWorker::HVal()
{
    return static_cast<float>(abs(_loc.x - _goalLoc.x) + abs(_loc.y - _goalLoc.y));
}


void AStar::SetTerrainMap(TileMap& map)
{
    _map = &map;

    _dims = _map->GetDims();

    // Construct our local copy of the map in a format suitable for the algo.
    // This consists of one "worker thread" per terrain tile
    _workers.resize(_dims.x * _dims.y);

    // The algorithm is implemented such that we will do N iterations,
    // where N is the shortest path from Start to Goal
    // At every iteration, any tiles on the boundary of our explored region will
    // update their distance heuristic from their 'closest' explored neighbor
    // The synchronization point ensures that all threads are beginning each iteration
    // in lockstep
    _syncPoint = std::make_shared<MyBarrier>(_workers.size());

    int n = 0;
    for (auto& w : _workers) {
        const int i = n % _dims.x;
        const int j = n / _dims.y;
        w._loc = {i, j};
        w._gridDims = _dims;
        /// TODO: assign effort from terrain type
        w._syncPoint = _syncPoint;
        w._goalReached = &_goalReached;
        w._thread = std::thread(&TileWorker::ThreadLoop, &w);

        n++;
    }
}

bool AStar::ComputePath(olc::vi2d start, olc::vi2d goal)
{
    _goalReached = false;

    for (auto& w : _workers) w.Reset();

    int sInd = start.y * _dims.x + start.x;
    int gInd = goal.y * _dims.x + goal.x;
    std::cout << "Start: " << sInd << " | Goal: " << gInd << std::endl;

    _workers[sInd]._isStart = true;
    _workers[gInd]._isGoal = true;

    for (auto& w : _workers) w.Start();

    while (!_goalReached) { usleep(1000); };

    return true;
}
