#include "MovementSystem.hpp"
#include "TransformComponent.hpp"
#include "SnakeComponent.hpp"
#include "Registry.hpp"

MovementSystem::MovementSystem(Registry& registry) : registry(registry) {}

void MovementSystem::update(float deltaTime) {
    for (auto entity : registry.getEntitiesWithComponents<TransformComponent, SnakeComponent>()) {
        auto& transform = registry.getComponent<TransformComponent>(entity);
        auto& snake = registry.getComponent<SnakeComponent>(entity);

        // Update position based on direction and speed
        transform.position.x += snake.direction.x * snake.speed * deltaTime;
        transform.position.y += snake.direction.y * snake.speed * deltaTime;

        // Handle boundary conditions (wrap around or clamp)
        if (transform.position.x < 0) {
            transform.position.x = arenaWidth; // Wrap around
        } else if (transform.position.x > arenaWidth) {
            transform.position.x = 0; // Wrap around
        }

        if (transform.position.y < 0) {
            transform.position.y = arenaHeight; // Wrap around
        } else if (transform.position.y > arenaHeight) {
            transform.position.y = 0; // Wrap around
        }
    }
}