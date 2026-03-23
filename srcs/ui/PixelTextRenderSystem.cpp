#include "PixelTextRenderSystem.hpp"
#include <raylib.h>
#include "../components/PixelTextComponent.hpp"
#include "../components/PixelTextLayoutComponent.hpp"

void PixelTextRenderSystem::render(Registry& registry) const {
    auto view = registry.view<PixelTextComponent, PixelTextLayoutComponent>();
    for (auto entity : view) {
        const auto& txt = registry.getComponent<PixelTextComponent>(entity);
        const auto& lay = registry.getComponent<PixelTextLayoutComponent>(entity);
        if (!txt.visible) continue;

        for (const auto& quad : lay.quads) {
            DrawRectangleRec(quad.rect, quad.color);
        }
    }
}