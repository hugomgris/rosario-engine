#include "AISystem.hpp"
#include "AIComponent.hpp"
#include "Registry.hpp"
#include "GameConfig.hpp"
#include "AIConfig.hpp"

void AISystem::update(Registry& registry, const GameConfig& gameConfig, const AIConfig& aiConfig) {
    for (auto entity : registry.getEntitiesWith<AIComponent>()) {
        auto& aiComponent = registry.getComponent<AIComponent>(entity);
        auto& transformComponent = registry.getComponent<TransformComponent>(entity);
        auto& snakeComponent = registry.getComponent<SnakeComponent>(entity);

        // Implement AI behavior based on the AIComponent state
        switch (aiComponent.state) {
            case AIState::Chase:
                // Logic for chasing the player or food
                chase(transformComponent, snakeComponent, aiConfig);
                break;
            case AIState::Flee:
                // Logic for fleeing from a threat
                flee(transformComponent, snakeComponent, aiConfig);
                break;
            case AIState::Wander:
                // Logic for wandering around
                wander(transformComponent, snakeComponent, aiConfig);
                break;
            default:
                break;
        }
    }
}

void AISystem::chase(TransformComponent& transform, SnakeComponent& snake, const AIConfig& aiConfig) {
    // Implement chasing logic
}

void AISystem::flee(TransformComponent& transform, SnakeComponent& snake, const AIConfig& aiConfig) {
    // Implement fleeing logic
}

void AISystem::wander(TransformComponent& transform, SnakeComponent& snake, const AIConfig& aiConfig) {
    // Implement wandering logic
}