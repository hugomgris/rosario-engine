#pragma once

#include <vector>
#include <raylib.h>

struct PixelTextLayoutComponent {
    std::vector<Rectangle>  quads;
    bool                    dirty = true;
};