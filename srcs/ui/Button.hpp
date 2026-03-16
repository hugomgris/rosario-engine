#pragma once

#include <string>
#include <functional>
#include <raylib.h>

struct Button {
    Rectangle           bounds          = { 0, 0, 0, 0 };
    std::string         label;

    Color               outlineColor    = { 70, 130, 180, 255 };
    Color               backgroundColor = { 23, 23, 23, 255 };
    Color               hoverColor      = { 255, 248, 227, 255 };
    Color               outlineHoverColor = { 255, 248, 227, 255 };
    Color               textColor       = { 255, 248, 227, 255 };
    Color               textHoverColor  = { 23, 23, 23, 255 };

    std::function<void()> onClick;

    bool isHovered(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, bounds);
    }

    void render(Font& font, bool hovered) const {
        Color bg      = hovered ? hoverColor      : backgroundColor;
        Color outline = hovered ? outlineHoverColor : outlineColor;
        Color text    = hovered ? textHoverColor  : textColor;

        DrawRectangleRec(bounds, bg);
        DrawRectangleLinesEx(bounds, 2.0f, outline);

        Vector2 textSize = MeasureTextEx(font, label.c_str(), 24.0f, 1.0f);
        float tx = bounds.x + (bounds.width  - textSize.x) / 2.0f;
        float ty = bounds.y + (bounds.height - textSize.y) / 2.0f;
        DrawTextEx(font, label.c_str(), { tx, ty }, 24.0f, 1.0f, text);
    }
};
