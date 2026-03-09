#ifndef RAYLIBCOLORS_HPP
#define RAYLIBCOLORS_HPP

#include "raylib.h"

namespace RaylibColors {
    const Color RED = { 255, 0, 0, 255 };
    const Color GREEN = { 0, 255, 0, 255 };
    const Color BLUE = { 0, 0, 255, 255 };
    const Color YELLOW = { 255, 255, 0, 255 };
    const Color WHITE = { 255, 255, 255, 255 };
    const Color BLACK = { 0, 0, 0, 255 };
    const Color ORANGE = { 255, 165, 0, 255 };
    const Color PURPLE = { 128, 0, 128, 255 };
    const Color CYAN = { 0, 255, 255, 255 };
    const Color MAGENTA = { 255, 0, 255, 255 };

    Color GetColorFromHex(const char* hex);
}

#endif // RAYLIBCOLORS_HPP