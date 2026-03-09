#pragma once
#include "DataStructs.hpp"
#include "ecs/Registry.hpp"

class MovementSystem {
public:
    void update(Registry& registry, float deltaTime);

private:
    void processInput(Registry& registry);
    void advanceSnake(Registry& registry, float deltaTime);

    Vec2 directionToVec2(Direction dir) const;
};
