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

#define SQRT2 1.41421356f

// Manhattan Distance
float Manhattan(const olc::vi2d& t1, const olc::vi2d& t2)
{
    const int dx = abs(t1.x - t2.x);
    const int dy = abs(t1.y - t2.y);
    return static_cast<float>(dx + dy);
}

// 'Diagonal Distance' (Straight lines and diagonals allowed)
float Diagonal(const olc::vi2d& t1, const olc::vi2d& t2)
{
    // Here we assume we follow a 45deg diagonal,
    // then a straignt line
    const int dx = abs(t1.x - t2.x);
    const int dy = abs(t1.y - t2.y);
    const int mind = std::min(dx, dy);
    const int maxd = std::max(dx, dy);
    return SQRT2 * mind + (maxd - mind);
}

float AStar::Hval(olc::vi2d t1, olc::vi2d t2)
{
    //return Manhattan(t1, t2);
    return Diagonal(t1, t2);
}


void AStar::SetTerrainMap(TileMap& map)
{
    _map = &map;

    /// TODO / HACK / NOTE: 
    //  We have an extra row+col of terrain tiles
    //  in order to fully fill the screen given how
    //  our map is generated
    _dims = _map->GetDims();
    _dims.x -= 1;
    _dims.y -= 1;

    // Construct our local copy of the map in a format suitable for the algo.
    // This consists of one "worker thread" per terrain tile
    _tiles.resize(_dims.x * _dims.y);

    int cidx = 0;
    for (auto& tile : _tiles) {
        const int i = cidx % _dims.x;
        const int j = cidx / _dims.x;
        tile.loc = {i, j};
        tile.idx = cidx;
        tile.effort = _map->GetEffortAt(i, j);

        /// DEBUGGING printf("(%d, %d): Effort %.1f\n", i, j, tile.effort);

        /* Setup neighbors list */
        if (tile.effort >= 0) {

            // Number of neighors with current alg.
            // TOP, BOTTOM, LEFT, RIGHT
            /// TODO: Allow corners (NN == 8); change HVal()
            // const int NN = 4;
            // int nidx[NN] = {cidx - _dims.x, cidx + _dims.x, cidx - 1, cidx + 1};
            // T/B/L/R; TL/TR/BL/BR
            const int NN = 8;
            int nidx[NN] = {
                cidx - _dims.x, cidx + _dims.x, cidx - 1, cidx + 1,
                cidx - _dims.x - 1, cidx - _dims.x + 1,
                cidx + _dims.x - 1, cidx + _dims.x + 1
            };

            for (int n = 0; n < NN; n++) {
                const int ni = nidx[n] % _dims.x;
                const int nj = nidx[n] / _dims.x;
                if (_map->GetEffortAt(ni, nj) >= 0) {
                    tile.neighbors.push_back(nidx[n]);
                }
            }

        } else {
            // Barrier / Impassable
        }

        cidx++;
    }
}

bool AStar::ComputePath(olc::vi2d start, olc::vi2d goal)
{
    int sInd = start.y * _dims.x + start.x;
    int gInd = goal.y * _dims.x + goal.x;

    if (sInd < 0 || sInd > _tiles.size()) return false;
    if (gInd < 0 || gInd > _tiles.size()) return false;

    /// DEBUGGING
    // printf("===Begin A*===\nStart/Goal: (%d,%d), (%d,%d)\n",
    //         start.x, start.y, goal.x, goal.y);

    // Reset all 'f' and 'g' scores for the A* alg.
    for (auto& tile : _tiles) {
        tile.f = FLT_MAX;
        tile.g = FLT_MAX;
    }

    // Start the algorithm with the start tile
    _tiles[sInd].g = 0;
    _tiles[sInd].f = Hval(start, goal);

    // Setup the priority queue to track the active/'open' tiles
    std::set<std::tuple<float, int, int> > pqueue;
    std::set<int> open_set;
    pqueue.insert(_tiles[sInd].GetTuple());
    open_set.insert(sInd);

    std::map<int, int> tree;
    int counter = 0;

    while (!pqueue.empty()) {
        auto tup = *(pqueue.begin());
        const int id = std::get<2>(tup);
        pqueue.erase(pqueue.begin());
        open_set.erase(id);
        ATile& current = _tiles[id];
        //printf("--current: %d (%d,%d), %.1f\n", id, current.loc.x, current.loc.y, current.effort);
        
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

        for (auto nidx : current.neighbors) {
            // Get the cost to traverse this neighbor
            auto& neighbor = _tiles[nidx];
            float tmp_g = current.g + Hval(current.loc, neighbor.loc) + neighbor.effort;
            /// DEBUGGING
            // printf("n %d (%d,%d): effort %.1f\n",
            //         nidx, neighbor.loc.x, neighbor.loc.y, neighbor.effort);

            if (tmp_g < neighbor.g) {
                // If this is the 'best' neighbor so far, update our score
                tree[nidx] = current.idx;
                neighbor.g = tmp_g;
                neighbor.f = tmp_g + Hval(neighbor.loc, goal);
                const int idx = neighbor.loc.x + neighbor.loc.y * _dims.x;

                if (open_set.count(idx) == 0) {
                    // Insert the neighbor into our set of spots to check
                    // Note that the 'f' score determines the priority in the queue
                    counter += 1;
                    neighbor.counter = counter;
                    pqueue.insert(neighbor.GetTuple());
                    open_set.insert(idx);
                    neighbor.state = ATile::State::OPEN;
                }
            }
        }
    }

    return false;
}
