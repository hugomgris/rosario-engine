#include "AI/GridHelper.hpp"

std::vector<Vec2> GridHelper::getNeighbors(Vec2 pos, int gridWidth, int gridHeight) const {
    std::vector<Vec2> result;
    result.reserve(4);
    if (pos.x > 0)              result.push_back({ pos.x - 1, pos.y });
    if (pos.x < gridWidth - 1)  result.push_back({ pos.x + 1, pos.y });
    if (pos.y > 0)              result.push_back({ pos.x,     pos.y - 1 });
    if (pos.y < gridHeight - 1) result.push_back({ pos.x,     pos.y + 1 });
    return result;
}

int GridHelper::manhattanDistance(Vec2 a, Vec2 b) const {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool GridHelper::isWalkable(const std::vector<std::vector<bool>>& blocked,
                             Vec2 pos,
                             int gridWidth,
                             int gridHeight,
                             const std::vector<Vec2>& ignorePositions) const {
    if (pos.x < 0 || pos.x >= gridWidth || pos.y < 0 || pos.y >= gridHeight)
        return false;

    for (const auto& ign : ignorePositions)
        if (pos.x == ign.x && pos.y == ign.y) return true;

    return !blocked[pos.x][pos.y];
}
