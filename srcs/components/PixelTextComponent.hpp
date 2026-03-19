#pragma once

#include <cstddef>
#include <unordered_map>
#include <string>
#include <raylib.h>

struct GlyphLigatureOverride {
    size_t      length = 2;
    std::string glyph;
};

struct PixelTextComponent {
    std::string     id;
    std::string     text;
    Vector2         position {0, 0};
    float           scale = 1.0f;
    Color           color = {255, 255, 255, 255};
    std::unordered_map<size_t, std::string> glyphOverrides;
    std::unordered_map<size_t, GlyphLigatureOverride> glyphLigatures;
    bool            centerX = false;
    bool            visible = true;
};