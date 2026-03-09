#include "TextSystem.hpp"
#include "Renderer.hpp"
#include "ecs/Registry.hpp"
#include "components/RenderComponent.hpp"
#include "components/TransformComponent.hpp"
#include <string>
#include <vector>

class TextSystem {
public:
    TextSystem(Renderer& renderer) : renderer(renderer) {}

    void renderText(const Registry& registry) {
        for (const auto& entity : registry.getEntitiesWith<RenderComponent, TransformComponent>()) {
            const auto& renderComponent = registry.getComponent<RenderComponent>(entity);
            const auto& transformComponent = registry.getComponent<TransformComponent>(entity);

            if (renderComponent.isText) {
                renderer.drawText(renderComponent.text, transformComponent.position.x, transformComponent.position.y, renderComponent.fontSize, renderComponent.color);
            }
        }
    }

private:
    Renderer& renderer;
};