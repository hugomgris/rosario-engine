#pragma once

#include <vector>
#include "ecs/Registry.hpp"
#include "AI/Pathfinder.hpp"
#include "AI/FloodFill.hpp"
#include "AI/GridHelper.hpp"
#include "arena/ArenaGrid.hpp"
#include "DataStructs.hpp"

using BlockedGrid = std::vector<std::vector<bool>>;

class AISystem {
public:
    AISystem(int gridWidth, int gridHeight)
        : gridWidth(gridWidth), gridHeight(gridHeight) {}

    // arena may be nullptr — falls back to SolidTag scan
    void update(Registry& registry, const ArenaGrid* arena = nullptr);

private:
    int         gridWidth;
    int         gridHeight;
    Pathfinder  pathfinder;
    FloodFill   floodFill;
    GridHelper  gridHelper;

    // Builds blocked[x][y] from ArenaGrid (preferred) or SolidTag entities + all snake bodies.
    BlockedGrid buildBlockedGrid(Registry& registry, const ArenaGrid* arena) const;

    Direction decideDirection(Registry& registry, Entity entity, const BlockedGrid& blocked);
    Direction goToFood      (Registry& registry, Entity entity, const BlockedGrid& blocked);
    Direction survivalMode  (Registry& registry, Entity entity, const BlockedGrid& blocked);
    Direction maximizeSpace (Registry& registry, Entity entity, const BlockedGrid& blocked);

    bool      isSafeMove    (const BlockedGrid& blocked, Vec2 nextPos) const;
    Direction vecToDirection(Vec2 from, Vec2 to) const;
    Vec2      stepInDirection(Vec2 pos, Direction dir) const;
};
