#include "CollisionSystem.hpp"
#include "Registry.hpp"
#include "CollisionComponent.hpp"
#include "TransformComponent.hpp"
#include "SnakeComponent.hpp"
#include "FoodComponent.hpp"

CollisionSystem::CollisionSystem(Registry& registry) : registry(registry) {}

void CollisionSystem::update() {
    for (auto entity : registry.getEntitiesWith<CollisionComponent, TransformComponent>()) {
        auto& collisionComp = registry.getComponent<CollisionComponent>(entity);
        auto& transformComp = registry.getComponent<TransformComponent>(entity);

        // Check for collisions with other entities
        for (auto otherEntity : registry.getEntitiesWith<CollisionComponent, TransformComponent>()) {
            if (entity == otherEntity) continue; // Skip self-collision

            auto& otherCollisionComp = registry.getComponent<CollisionComponent>(otherEntity);
            auto& otherTransformComp = registry.getComponent<TransformComponent>(otherEntity);

            if (checkCollision(transformComp, otherTransformComp)) {
                handleCollision(entity, otherEntity);
            }
        }
    }
}

bool CollisionSystem::checkCollision(const TransformComponent& a, const TransformComponent& b) {
    return (a.position.x < b.position.x + b.size.x &&
            a.position.x + a.size.x > b.position.x &&
            a.position.y < b.position.y + b.size.y &&
            a.position.y + a.size.y > b.position.y);
}

void CollisionSystem::handleCollision(Entity entityA, Entity entityB) {
    // Handle collision response based on the types of entities involved
    auto& snakeComp = registry.getComponent<SnakeComponent>(entityA);
    auto& foodComp = registry.getComponent<FoodComponent>(entityB);

    if (snakeComp && foodComp) {
        // Snake eats food
        registry.destroyEntity(entityB); // Remove food entity
        snakeComp.length++; // Increase snake length
    }
    // Additional collision handling logic can be added here
}