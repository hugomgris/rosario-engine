#pragma once

#include <string>
#include <unordered_map>
#include "../components/AIComponent.hpp"

class AIPresetLoader {
public:
    using PresetTable = std::unordered_map<std::string, AIComponent>;

    static PresetTable load(const std::string& path);
};
