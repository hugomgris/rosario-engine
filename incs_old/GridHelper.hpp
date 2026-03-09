#pragma once

#include "DataStructs.hpp"
#include "Snake.hpp"
#include <vector>
#include <cmath>

class GridHelper {
protected:
    static bool isWalkable(const GameState& state, Vec2 pos, const std::vector<Vec2>& ignorePositions = {});
    static std::vector<Vec2> getNeighbors(Vec2 pos);
    static int manhattanDistance(Vec2 a, Vec2 b);
};