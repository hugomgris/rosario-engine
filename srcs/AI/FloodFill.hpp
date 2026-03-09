#pragma once

#include <vector>
#include "GridHelper.hpp"
#include "ecs/Registry.hpp"
#include "DataStructs.hpp"

class FloodFill {
public:
    int countReachable(const std::vector<std::vector<bool>>& blocked,
                       Vec2 start,
                       int gridWidth,
                       int gridHeight,
                       const std::vector<Vec2>& ignorePositions = {}) const;

    // canReachTail still needs the registry to read the snake's tail position.
    bool canReachTail(const Registry& registry,
                      const std::vector<std::vector<bool>>& blocked,
                      Entity aiEntity,
                      const std::vector<Vec2>& proposedPath,
                      int gridWidth,
                      int gridHeight) const;

private:
    GridHelper gridHelper;
};
