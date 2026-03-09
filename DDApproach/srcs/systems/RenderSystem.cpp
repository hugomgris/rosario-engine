#include "RenderSystem.hpp"
#include "RenderComponent.hpp"
#include "ecs/Registry.hpp"
#include "graphics/Renderer.hpp"

RenderSystem::RenderSystem(Registry& registry, Renderer& renderer)
    : registry(registry), renderer(renderer) {}

void RenderSystem::update(float deltaTime) {
    for (const auto& entity : registry.getEntitiesWith<RenderComponent>()) {
        const auto& renderComponent = registry.getComponent<RenderComponent>(entity);
        const auto& transformComponent = registry.getComponent<TransformComponent>(entity);

        if (renderComponent.visible) {
            renderer.draw(renderComponent.texture, transformComponent.position, transformComponent.rotation, transformComponent.scale);
        }
    }
}