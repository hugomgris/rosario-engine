#pragma once

#include <vector>
#include "DataStructs.hpp"

class GridHelper {
public:
    // Returns the four cardinal neighbours of pos that are in-bounds.
    std::vector<Vec2> getNeighbors(Vec2 pos, int gridWidth, int gridHeight) const;

    // Manhattan distance heuristic
    int manhattanDistance(Vec2 a, Vec2 b) const;

    // Returns true if pos is walkable according to a pre-built blocked grid.
    // blocked[x][y] == true means the cell is occupied/solid.
    // ignorePositions are forced walkable (tail prediction).
    bool isWalkable(const std::vector<std::vector<bool>>& blocked,
                    Vec2 pos,
                    int gridWidth,
                    int gridHeight,
                    const std::vector<Vec2>& ignorePositions = {}) const;
};
