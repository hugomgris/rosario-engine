#pragma once

#include <string>
#include <unordered_map>
#include "../systems/PostProcessingSystem.hpp"

class PostProcessConfigLoader {
    public:
        using PresetTable = std::unordered_map<std::string, PostProcessConfig>;

        static PresetTable load(const std::string& path);
};
