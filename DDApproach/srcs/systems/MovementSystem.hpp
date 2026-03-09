#ifndef MOVEMENTSYSTEM_HPP
#define MOVEMENTSYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/TransformComponent.hpp"
#include "components/SnakeComponent.hpp"

class MovementSystem {
public:
    void update(Registry& registry, float deltaTime);

private:
    void moveSnake(Entity entity, TransformComponent& transform, SnakeComponent& snake, float deltaTime);
};

#endif // MOVEMENTSYSTEM_HPP