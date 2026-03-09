#ifndef AISYSTEM_HPP
#define AISYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/AIComponent.hpp"
#include "components/TransformComponent.hpp"
#include "AI/Pathfinder.hpp"
#include "data/AIConfig.hpp"

class AISystem {
public:
    AISystem(Registry& registry, const AIConfig& aiConfig);

    void update(float deltaTime);

private:
    Registry& registry;
    const AIConfig& aiConfig;
    Pathfinder pathfinder;

    void processAIComponent(Entity entity, AIComponent& aiComponent, TransformComponent& transformComponent);
};

#endif // AISYSTEM_HPP