#pragma once

#include <vector>
#include "AI/GridHelper.hpp"
#include "AI/AITypes.hpp"
#include "DataStructs.hpp"

class Pathfinder {
public:
    std::vector<Vec2> findPath(const std::vector<std::vector<bool>>& blocked,
                               Vec2 start,
                               Vec2 goal,
                               int gridWidth,
                               int gridHeight,
                               int maxDepth,
                               const std::vector<Vec2>& ignorePositions = {}) const;

private:
    GridHelper          gridHelper;
    std::vector<Vec2>   reconstructPath(Node* goalNode) const;
    void                cleanNodes(std::vector<Node*>& nodes) const;
};
