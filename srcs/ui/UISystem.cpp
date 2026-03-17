#include "UISystem.hpp"

void UISystem::renderRects(UIRenderQueue& queue) const {
    for (auto cmd : queue.rects) {
        if (cmd.outline) {
            DrawRectangleLinesEx(cmd.rect, cmd.lineThickness, cmd.color);
        } else {
            DrawRectangleRec(cmd.rect, cmd.color);
        }
    }
}