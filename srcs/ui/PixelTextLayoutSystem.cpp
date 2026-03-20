#include "PixelTextLayoutSystem.hpp"
#include <algorithm>
#include <limits>
#include "../components/PixelTextComponent.hpp"
#include "../components/PixelTextLayoutComponent.hpp"

namespace {
    int resolveAdvanceCells(const GlyphDef& glyph) {
        if (glyph.advance > 0) {
            return glyph.advance;
        }

        if (glyph.cells.empty()) {
            return 1;
        }

        int minX = glyph.cells.front().x;
        int maxX = glyph.cells.front().x;
        for (const auto& cell : glyph.cells) {
            minX = std::min(minX, cell.x);
            maxX = std::max(maxX, cell.x);
        }

        return (maxX - minX) + 1;
    }
}

void PixelTextLayoutSystem::update(Registry& registry, const GlyphLibrary& lib) {
    auto view = registry.view<PixelTextComponent, PixelTextLayoutComponent>();
    const float cellPixel = static_cast<float>(lib.cellSize);
    const float letterSpacingPixel = static_cast<float>(lib.letterSpacing);
    const float lineAdvanceCells = static_cast<float>(lib.resolvedLineAdvanceCells());

    for (auto entity : view) {
        auto& text = registry.getComponent<PixelTextComponent>(entity);
        auto& layout = registry.getComponent<PixelTextLayoutComponent>(entity);
        if (!layout.dirty) continue;

        layout.quads.clear();
        float cursorX = text.position.x;
        float cursorY = text.position.y;
        const float cell = cellPixel * text.scale;
        size_t glyphIndex = 0;
        bool atLineStart = true;
        bool hadLeadingSpace = false;

        for (size_t textIndex = 0; textIndex < text.text.size();) {
            char ch = text.text[textIndex];
            if (ch == '\n') {
                cursorX = text.position.x;
                cursorY += (lineAdvanceCells * cellPixel) * text.scale;
                ++glyphIndex;
                ++textIndex;
                atLineStart = true;
                hadLeadingSpace = false;
                continue;
            }

            const GlyphDef* gDef = nullptr;
            size_t consumedChars = 1;

            auto ligatureIt = text.glyphLigatures.find(glyphIndex);
            if (ligatureIt != text.glyphLigatures.end()) {
                const auto& ligature = ligatureIt->second;
                if (ligature.length > 1 && (textIndex + ligature.length) <= text.text.size()) {
                    bool spansLineBreak = false;
                    for (size_t i = 0; i < ligature.length; ++i) {
                        if (text.text[textIndex + i] == '\n') {
                            spansLineBreak = true;
                            break;
                        }
                    }

                    if (!spansLineBreak) {
                        const GlyphDef* ligatureGlyph = lib.findById(ligature.glyph);
                        if (ligatureGlyph) {
                            gDef = ligatureGlyph;
                            consumedChars = ligature.length;
                        }
                    }
                }
            }

            auto overrideIt = text.glyphOverrides.find(glyphIndex);
            if (!gDef && overrideIt != text.glyphOverrides.end()) {
                gDef = lib.findById(overrideIt->second);
            }
            if (!gDef) {
                gDef = lib.find(ch);
            }

            Color glyphColor = text.color;
            auto glyphColorIt = text.glyphColorOverrides.find(glyphIndex);
            if (glyphColorIt != text.glyphColorOverrides.end()) {
                glyphColor = glyphColorIt->second;
            }

            const std::vector<GlyphCellColorOverride>* cellOverrides = nullptr;
            auto cellOverrideIt = text.glyphCellColorOverrides.find(glyphIndex);
            if (cellOverrideIt != text.glyphCellColorOverrides.end()) {
                cellOverrides = &cellOverrideIt->second;
            }

            if (atLineStart && ch == ' ') {
                hadLeadingSpace = true;
            }

            if (!gDef) {
                cursorX += (cellPixel + letterSpacingPixel) * text.scale;
                textIndex += consumedChars;
                glyphIndex += consumedChars;
                continue;
            }

            if (atLineStart && hadLeadingSpace && ch != ' ') {
                cursorX += letterSpacingPixel * text.scale;
            }

            for (const auto& c : gDef->cells) {
                Color cellColor = glyphColor;
                if (cellOverrides) {
                    for (const auto& overrideCell : *cellOverrides) {
                        if (overrideCell.x == c.x && overrideCell.y == c.y) {
                            cellColor = overrideCell.color;
                            break;
                        }
                    }
                }

                layout.quads.push_back(PixelTextQuad{
                    Rectangle {
                        cursorX + (static_cast<float>(c.x + gDef->offsetX) * cell),
                        cursorY + (static_cast<float>(c.y + gDef->offsetY) * cell),
                        cell,
                        cell
                    },
                    cellColor
                });
            }

            const float advanceCells = static_cast<float>(resolveAdvanceCells(*gDef));
            cursorX += ((advanceCells * cellPixel) + letterSpacingPixel) * text.scale;
            textIndex += consumedChars;
            glyphIndex += consumedChars;

            if (ch != ' ') {
                atLineStart = false;
            }
        }
        
        if (text.centerX && !layout.quads.empty()) {
            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();

            for (const auto& quad : layout.quads) {
                minX = std::min(minX, quad.rect.x);
                maxX = std::max(maxX, quad.rect.x + quad.rect.width);
            }

            const float currentCenterX = (minX + maxX) * 0.5f;
            const float deltaX = text.position.x - currentCenterX;
            for (auto& quad : layout.quads) {
                quad.rect.x += deltaX;
            }
        }

        layout.dirty = false;
    }
}