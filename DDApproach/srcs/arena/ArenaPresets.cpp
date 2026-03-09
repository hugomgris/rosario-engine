#include "ArenaPresets.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ArenaPresets::ArenaPresets(const std::string& configFilePath) {
    loadPresets(configFilePath);
}

void ArenaPresets::loadPresets(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open presets configuration file.");
    }

    json j;
    file >> j;

    for (const auto& item : j) {
        ArenaConfig config;
        config.width = item["width"];
        config.height = item["height"];
        config.wallType = item["wallType"];
        // Load other configuration parameters as needed

        presets.push_back(config);
    }
}

const std::vector<ArenaConfig>& ArenaPresets::getPresets() const {
    return presets;
}