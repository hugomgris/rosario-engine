#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class ArenaPreset {
public:
    std::string name;
    std::vector<std::vector<int>> layout; // 2D grid representation of the arena
    int width;
    int height;

    ArenaPreset(const std::string& name, const std::vector<std::vector<int>>& layout, int width, int height)
        : name(name), layout(layout), width(width), height(height) {}
};

class ArenaPresets {
public:
    static std::vector<ArenaPreset> loadPresets(const std::string& filePath);
};