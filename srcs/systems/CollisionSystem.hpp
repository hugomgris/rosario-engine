#pragma once

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"

class CollisionSystem {
public:
    // arena may be nullptr — falls back to SolidTag entity scan for compatibility.
    void update(Registry& registry, const ArenaGrid* arena = nullptr);

private:
    void clearResults(Registry& registry);
    void checkWallCollisions(Registry& registry, const ArenaGrid* arena);
    void checkSelfCollisions(Registry& registry);
    void checkSnakeCollisions(Registry& registry);
    void checkFoodCollisions(Registry& registry);
};
