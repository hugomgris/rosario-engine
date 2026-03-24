#include "ArenaPresetLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <cstdlib>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

static void validateMaskRow(const std::string& row, const std::string& presetName, int rowIndex) {
    for (size_t i = 0; i < row.size(); ++i) {
        const char c = row[i];
        if (c != '.' && c != '#') {
            throw std::runtime_error(
                "ArenaPresetLoader: invalid wall token in preset '" + presetName +
                "' at row " + std::to_string(rowIndex) + ", col " + std::to_string(i) +
                ": expected '.' or '#'");
        }
    }
}

ArenaPresetLoader::PresetList ArenaPresetLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("ArenaPresetLoader: cannot open: " + path);

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("ArenaPresetLoader: parse error: " + std::string(e.what()));
    }

    PresetList presets;
    for (const auto& entry : data.at("presets")) {
        ArenaPresetDefinition preset;
        preset.name = entry.at("name").get<std::string>();
        preset.width = entry.at("width").get<int>();
        preset.height = entry.at("height").get<int>();
        preset.walls = entry.at("walls").get<std::vector<std::string>>();

        if (preset.width <= 0 || preset.height <= 0) {
            throw std::runtime_error("ArenaPresetLoader: invalid preset dimensions for '" + preset.name + "'");
        }

        if (static_cast<int>(preset.walls.size()) != preset.height) {
            throw std::runtime_error(
                "ArenaPresetLoader: height mismatch in preset '" + preset.name + "': expected " +
                std::to_string(preset.height) + ", got " + std::to_string(preset.walls.size()));
        }

        for (int y = 0; y < preset.height; ++y) {
            if (static_cast<int>(preset.walls[y].size()) != preset.width) {
                throw std::runtime_error(
                    "ArenaPresetLoader: width mismatch in preset '" + preset.name + "' at row " +
                    std::to_string(y) + ": expected " + std::to_string(preset.width) +
                    ", got " + std::to_string(preset.walls[y].size()));
            }
            validateMaskRow(preset.walls[y], preset.name, y);
        }

        presets.push_back(preset);
    }

    return presets;
}

const ArenaPresetDefinition& ArenaPresetLoader::pickRandom(const PresetList& presets) {
    if (presets.empty()) {
        throw std::runtime_error("ArenaPresetLoader: cannot pick random preset from empty list");
    }

    const size_t index = static_cast<size_t>(std::rand()) % presets.size();
    return presets[index];
}
