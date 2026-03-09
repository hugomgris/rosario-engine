#include "AnimationSystem.hpp"
#include "ecs/Registry.hpp"
#include "components/TransformComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "graphics/Renderer.hpp"

AnimationSystem::AnimationSystem(Registry& registry, Renderer& renderer)
    : registry(registry), renderer(renderer) {}

void AnimationSystem::update(float deltaTime) {
    for (auto entity : registry.view<TransformComponent, RenderComponent>()) {
        auto& transform = registry.get<TransformComponent>(entity);
        auto& render = registry.get<RenderComponent>(entity);

        // Update animation state based on entity's state
        if (render.isAnimating) {
            render.animationTime += deltaTime;
            if (render.animationTime >= render.animationDuration) {
                render.animationTime = 0.0f;
                render.currentFrame = (render.currentFrame + 1) % render.totalFrames;
            }
        }
    }
}

void AnimationSystem::render() {
    for (auto entity : registry.view<TransformComponent, RenderComponent>()) {
        auto& transform = registry.get<TransformComponent>(entity);
        auto& render = registry.get<RenderComponent>(entity);

        if (render.isVisible) {
            renderer.drawSprite(render.sprite, transform.position, render.currentFrame);
        }
    }
}