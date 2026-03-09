#pragma once

#include <vector>
#include "ArenaConfig.hpp"
#include "ecs/Entity.hpp"

class Arena {
public:
    Arena(const ArenaConfig& config);
    void update();
    void render();
    void reset();

    std::vector<Entity> getWalls() const;
    std::vector<Entity> getFood() const;

private:
    void initializeWalls();
    void initializeFood();

    std::vector<Entity> walls;
    std::vector<Entity> food;
    ArenaConfig config;
};