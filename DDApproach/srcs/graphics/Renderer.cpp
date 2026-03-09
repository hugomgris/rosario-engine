#include "Renderer.hpp"
#include "RaylibColors.hpp"
#include "ecs/Registry.hpp"
#include "components/RenderComponent.hpp"
#include "components/TransformComponent.hpp"
#include <vector>

Renderer::Renderer() {}

void Renderer::init() {
    // Initialize rendering settings if needed
}

void Renderer::render(const Registry& registry) {
    for (const auto& entity : registry.getEntitiesWith<RenderComponent, TransformComponent>()) {
        const auto& renderComponent = registry.getComponent<RenderComponent>(entity);
        const auto& transformComponent = registry.getComponent<TransformComponent>(entity);

        // Set color based on the RenderComponent
        Color color = RaylibColors::getColor(renderComponent.color);
        
        // Draw the entity based on its type
        switch (renderComponent.type) {
            case RenderType::Sprite:
                DrawTexture(renderComponent.texture, transformComponent.position.x, transformComponent.position.y, color);
                break;
            case RenderType::Rectangle:
                DrawRectangle(transformComponent.position.x, transformComponent.position.y, renderComponent.width, renderComponent.height, color);
                break;
            // Add more cases for different render types as needed
        }
    }
}

void Renderer::clear() {
    ClearBackground(RAYWHITE);
}