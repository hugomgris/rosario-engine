#pragma once

#include "../ecs/Registry.hpp"

class PixelTextRenderSystem {
    public:
        void render(Registry& registry) const;
};