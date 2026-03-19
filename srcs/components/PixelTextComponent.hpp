#pragma once

#include <string>
#include <raylib.h>

struct PixelTextComponent {
    std::string     id;
    std::string     text;
    Vector2         position {0, 0};
    float           scale = 1.0f;
    Color           color = {255, 255, 255, 255};
    bool            visible = true;
};