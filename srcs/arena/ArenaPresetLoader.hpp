#pragma once

#include <string>
#include <vector>
#include "../arena/ArenaGrid.hpp"

class ArenaPresetLoader {
public:
    // Returns presets in JSON order — used for cycling with TAB
    static std::vector<WallPreset> load(const std::string& path);
};
