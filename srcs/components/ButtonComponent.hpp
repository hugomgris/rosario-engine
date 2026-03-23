#pragma once

#include <string>
#include <raylib.h>
#include "../ui/ButtonConfig.hpp"

struct ButtonComponent {
    std::string id;
    Rectangle bounds;
    ButtonConfig config;
    bool hovered = false;
};
