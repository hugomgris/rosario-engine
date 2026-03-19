#pragma once

#include "../ecs/Registry.hpp"
#include "GlyphLibrary.hpp"

class PixelTextLayoutSystem {
    public:
        void update(Registry& registry, const GlyphLibrary& lib);
};