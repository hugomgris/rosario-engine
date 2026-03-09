#include "InputSystem.hpp"
#include "InputManager.hpp"
#include "ecs/Registry.hpp"
#include "components/SnakeComponent.hpp"
#include "components/TransformComponent.hpp"

InputSystem::InputSystem(InputManager& inputManager, Registry& registry)
    : inputManager(inputManager), registry(registry) {}

void InputSystem::processInput() {
    if (inputManager.isKeyPressed(KEY_UP)) {
        updateSnakeDirection(Direction::UP);
    }
    if (inputManager.isKeyPressed(KEY_DOWN)) {
        updateSnakeDirection(Direction::DOWN);
    }
    if (inputManager.isKeyPressed(KEY_LEFT)) {
        updateSnakeDirection(Direction::LEFT);
    }
    if (inputManager.isKeyPressed(KEY_RIGHT)) {
        updateSnakeDirection(Direction::RIGHT);
    }
}

void InputSystem::updateSnakeDirection(Direction direction) {
    for (auto entity : registry.getEntitiesWith<SnakeComponent>()) {
        auto& snake = registry.getComponent<SnakeComponent>(entity);
        snake.direction = direction;
    }
}