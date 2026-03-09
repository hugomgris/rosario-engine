#pragma once

#include <vector>
#include "GridHelper.hpp"
#include "AIConfig.hpp"

class Pathfinder {
public:
    Pathfinder(const AIConfig& config);
    
    std::vector<Vector2> findPath(const std::vector<std::vector<CellType>>& grid, const Vector2& start, const Vector2& goal);
    
private:
    const AIConfig& config;

    std::vector<Vector2> reconstructPath(const std::vector<Vector2>& cameFrom, const Vector2& current);
    float heuristic(const Vector2& a, const Vector2& b);
};