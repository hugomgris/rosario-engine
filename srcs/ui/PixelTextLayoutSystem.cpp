#include "PixelTextLayoutSystem.hpp"
#include "../components/PixelTextComponent.hpp"
#include "../components/PixelTextLayoutComponent.hpp"

void PixelTextLayoutSystem::update(Registry& registry, const GlyphLibrary& lib) {
    auto view = registry.view<PixelTextComponent, PixelTextLayoutComponent>();
    const float cellPixel = static_cast<float>(lib.cellSize);
    const float letterSpacingPixel = static_cast<float>(lib.letterSpacing);
    const float lineSpacingPixel = static_cast<float>(lib.lineSpacing);
    const float lineHeightCells = static_cast<float>(lib.resolvedLineHeightCells());

    for (auto entity : view) {
        auto& text = registry.getComponent<PixelTextComponent>(entity);
        auto& layout = registry.getComponent<PixelTextLayoutComponent>(entity);
        if (!layout.dirty) continue;

        layout.quads.clear();
        float cursorX = text.position.x;
        float cursorY = text.position.y;
        const float cell = cellPixel * text.scale;

        for (char ch : text.text) {
            if (ch == '\n') {
                cursorX = text.position.x;
                cursorY += ((lineHeightCells * cellPixel) + lineSpacingPixel) * text.scale;
                continue;
            }
            
            const GlyphDef* gDef = lib.find(ch);
            if (!gDef) {
                cursorX += (cellPixel + letterSpacingPixel) * text.scale;
                continue;
            }

            for (const auto& c : gDef->cells) {
                layout.quads.push_back(Rectangle {
                    cursorX + c.x * cell,
                    cursorY + c.y * cell,
                    cell,
                    cell
                });
            }

            cursorX += ((static_cast<float>(gDef->advance) * cellPixel) + letterSpacingPixel) * text.scale;
        }
        
        layout.dirty = false;
    }
}