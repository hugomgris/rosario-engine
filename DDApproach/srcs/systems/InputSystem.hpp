#ifndef INPUTSYSTEM_HPP
#define INPUTSYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/TransformComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/CollisionComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/FoodComponent.hpp"
#include "core/InputManager.hpp"

class InputSystem {
public:
    InputSystem(Registry& registry, InputManager& inputManager);
    void processInput(float deltaTime);

private:
    Registry& registry;
    InputManager& inputManager;

    void handleSnakeInput(Entity snakeEntity);
    void updateTransform(Entity entity, const TransformComponent& transform);
};

#endif // INPUTSYSTEM_HPP