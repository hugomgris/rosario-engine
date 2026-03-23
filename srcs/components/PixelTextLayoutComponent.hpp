#pragma once

#include <vector>
#include <raylib.h>

struct PixelTextQuad {
    Rectangle rect;
    Color     color;
};

struct PixelTextLayoutComponent {
    std::vector<PixelTextQuad> quads;
    bool                       dirty = true;
};