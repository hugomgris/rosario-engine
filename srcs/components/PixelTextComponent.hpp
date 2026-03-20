#pragma once

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>
#include <raylib.h>
#include "../../incs/DataStructs.hpp"

struct GlyphLigatureOverride {
    size_t      length = 2;
    std::string glyph;
};

struct GlyphCellColorOverride {
    int     x = 0;
    int     y = 0;
    Color   color = {255, 255, 255, 255};
};

struct PixelTextComponent {
    std::string     id;
    std::string     text;
    Vector2         position {0, 0};
    float           scale = 1.0f;
    Color           color = {255, 255, 255, 255};
    std::unordered_map<size_t, std::string> glyphOverrides;
    std::unordered_map<size_t, GlyphLigatureOverride> glyphLigatures;
    std::unordered_map<size_t, Color> glyphColorOverrides;
    std::unordered_map<size_t, std::vector<GlyphCellColorOverride>> glyphCellColorOverrides;
    std::vector<GameState> visibleInStates;
    bool            centerX = false;
    bool            visible = true;
};