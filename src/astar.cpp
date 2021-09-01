/**
 * @File: threaded_astar.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Implements the A* algorithm for a 2D array of tiles
 */

#include "astar.hpp"

#include <cassert>
#include <map>
#include <set>
#include <unordered_set>
#include <unistd.h>

float AStar::Hval(olc::vi2d t1, olc::vi2d t2)
{
    const int dx = abs(t1.x - t2.x);
    const int dy = abs(t1.y - t2.y);
    return static_cast<float>(dx + dy);
}


void AStar::SetTerrainMap(TileMap& map)
{
    _map = &map;

    _dims = _map->GetDims();

    // Construct our local copy of the map in a format suitable for the algo.
    // This consists of one "worker thread" per terrain tile
    _tiles.resize(_dims.x * _dims.y);

    int n = 0;
    for (auto& tile : _tiles) {
        const int i = n % _dims.x;
        const int j = n / _dims.y;
        tile.loc = {i, j};
        tile.idx = n;
        tile.effort = 0.f; /// TODO: Assigned based on terrain type

        /* Setup neighbors list */
        /// TODO: Check for impassible barriers; tag and ignore

        // TOP - subtract one row
        if (j > 0) {
            ATile* tn = &_tiles[n - _dims.x];
            tile.neighbors.push_back(tn);
        }
        // BOTTOM - add one row
        if (j < _dims.y - 1) {
            ATile* tn = &_tiles[n + _dims.x];
            tile.neighbors.push_back(tn);
        }
        // LEFT - subtract one column
        if (i > 0) {
            ATile* tn = &_tiles[n - 1];
            tile.neighbors.push_back(tn);
        }
        // RIGHT - add one column
        if (i < _dims.x - 1) {
            ATile* tn = &_tiles[n + 1];
            tile.neighbors.push_back(tn);
        }

        n++;
    }
}

bool AStar::ComputePath(olc::vi2d start, olc::vi2d goal)
{
    int sInd = start.y * _dims.x + start.x;
    int gInd = goal.y * _dims.x + goal.x;
    std::cout << "Start: " << sInd << " | Goal: " << gInd << std::endl;

    // Reset all 'f' and 'g' scores for the A* alg.
    for (auto& tile : _tiles) {
        tile.f = FLT_MAX;
        tile.g = FLT_MAX;
    }

    // Start the algorithm with the start tile
    _tiles[sInd].g = 0;
    _tiles[sInd].f = Hval(start, goal);

    // Setup the priority queue to track the active/'open' tiles
    // The compare function shall return if t1 is less than t2
    auto tcomp = [](ATile* t1, ATile* t2) {
        if (t1 == t2)
            return false;

        if (t1->f == t2->f) {
            return t1->counter < t2->counter;
        }
        return t1->f < t2->f;
    };

    std::set<std::tuple<float, int, int> > pqueue;
    std::set<int> open_set;
    pqueue.insert(_tiles[sInd].GetTuple());
    open_set.insert(sInd);

    std::map<int, int> tree;
    std::set<ATile*> open_set_hash;
    int counter = 0;

    while (!pqueue.empty()) {
        auto tup = *(pqueue.begin());
        const int id = std::get<2>(tup);
        pqueue.erase(pqueue.begin());
        open_set.erase(id);
        ATile& current = _tiles[id];
        
        // Check to see if we've reached our destination 
        if (current.loc == goal) {
            /* --- A Path Was Found --- */
            _path_cost = current.g;

            // Save the path to be drawn later
            _final_path.clear();
            int idx = current.idx;
            while (tree.count(idx)) {
                auto loc = _tiles[idx].loc;
                _final_path.insert(_final_path.begin(), loc);
                idx = tree[idx];
            }

            return true;
        }

        for (auto neighbor : current.neighbors) {
            // Get the cost to traverse this neighbor
            float tmp_g = current.g + 1 + neighbor->effort;

            if (tmp_g < neighbor->g) {
                // If this is the 'best' neighbor so far, update our score
                tree[neighbor->idx] = current.idx;
                neighbor->g = tmp_g;
                neighbor->f = tmp_g + Hval(neighbor->loc, goal);
                const int idx = neighbor->loc.x + neighbor->loc.y * _dims.x;

                if (open_set.count(idx) == 0) {
                    // Insert the neighbor into our set of spots to check
                    // Note that the 'f' score determines the priority in the queue
                    counter += 1;
                    neighbor->counter = counter;
                    pqueue.insert(neighbor->GetTuple());
                    open_set.insert(idx);
                    neighbor->state = ATile::State::OPEN;
                }
            }
        }
    }

    return false;
}
