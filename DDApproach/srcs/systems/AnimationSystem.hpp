#ifndef ANIMATION_SYSTEM_HPP
#define ANIMATION_SYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/TransformComponent.hpp"
#include "components/RenderComponent.hpp"

class AnimationSystem {
public:
    void update(Registry& registry, float deltaTime);
    void playAnimation(Entity entity, const std::string& animationName);
    void stopAnimation(Entity entity);
    void setAnimationSpeed(Entity entity, float speed);
    
private:
    void updateAnimation(Entity entity, TransformComponent& transform, RenderComponent& render, float deltaTime);
    std::vector<std::string> getAvailableAnimations(Entity entity);
};

#endif // ANIMATION_SYSTEM_HPP