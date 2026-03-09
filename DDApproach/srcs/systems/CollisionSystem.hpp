#ifndef COLLISIONSYSTEM_HPP
#define COLLISIONSYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/CollisionComponent.hpp"
#include "components/TransformComponent.hpp"

class CollisionSystem {
public:
    void update(Registry& registry);

private:
    void handleCollisions(Registry& registry, Entity entityA, Entity entityB);
    bool checkCollision(const TransformComponent& transformA, const TransformComponent& transformB);
};

#endif // COLLISIONSYSTEM_HPP