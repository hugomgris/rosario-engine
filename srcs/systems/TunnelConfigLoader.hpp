#pragma once

#include <string>
#include <unordered_map>
#include "AnimationSystem.hpp"

class TunnelConfigLoader {
public:
    using PresetTable = std::unordered_map<std::string, TunnelConfig>;

    static PresetTable load(const std::string& path);
};
