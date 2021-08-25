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

    for (auto& tile : _tiles) {
        // Reset the 'f' and 'g' scores for the A* alg.
        tile.f = FLT_MAX;
        tile.g = FLT_MAX;
    }

    _tiles[sInd].g = 0;
    _tiles[sInd].f = Hval(start, goal);

    // Setup the priority queue
    auto tcomp = [](ATile* t1, ATile* t2) {
        if (t1->f == t2->f) {
            return t1->counter < t2->counter;
        }
        return t1->f < t2->f;
    };

    //std::priority_queue<ATile*, std::vector<ATile*>, decltype(tcomp)> pqueue(tcomp);
    std::set<ATile*, decltype(tcomp)> pqueue(tcomp);

    pqueue.insert(&_tiles[sInd]);

    std::map<ATile*, ATile*> tree;
    std::set<ATile*> open_set_hash;
    int counter = 0;

    while (!pqueue.empty()) {
        auto current = *(pqueue.begin());
        pqueue.erase(current);
        
        if (current->loc == goal) {
            std::cout << "Path found!" << std::endl;
            // Save the path to be drawn later
            auto tmp = current;
            while (tree.count(tmp)) {
                auto loc = tmp->loc;
                std::cout << "[" << loc.x << "," << loc.y << "]" << std::endl;
                tmp = tree[tmp];
            }
            return true;
        }

        for (auto neighbor : current->neighbors) {
            // Get the cost to traverse this neighbor
            float tmp_g = current->g + 1 + neighbor->effort;

            if (tmp_g < neighbor->g) {
                // If this is the 'best' neighbor so far, update our score
                tree[neighbor] = current;
                neighbor->g = tmp_g;
                neighbor->f = tmp_g + Hval(neighbor->loc, goal);

                if (pqueue.count(neighbor) == 0) {
                    // Insert the neighbor into our set of spots to check
                    // Note that the 'f' score determines the priority in the queue
                    counter += 1;
                    neighbor->counter = counter;
                    pqueue.insert(neighbor);
                    neighbor->state = ATile::State::OPEN;
                }
            }
        }
    }

    return false;
}
