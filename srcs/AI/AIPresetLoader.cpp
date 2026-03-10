#include "AI/AIPresetLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

static AIBehaviourState parseBehavior(const std::string& s) {
    if (s == "EASY")   return AIBehaviourState::EASY;
    if (s == "MEDIUM") return AIBehaviourState::MEDIUM;
    if (s == "HARD")   return AIBehaviourState::HARD;
    throw std::runtime_error("AIPresetLoader: unknown behavior: " + s);
}

AIPresetLoader::PresetTable AIPresetLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("AIPresetLoader: cannot open file: " + path);

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("AIPresetLoader: JSON parse error: " + std::string(e.what()));
    }

    PresetTable table;

    for (const auto& entry : data.at("AIPresets")) {
        AIComponent preset;
        preset.behavior         = parseBehavior(entry.at("behavior").get<std::string>());
        preset.maxSearchDepth   = entry.at("maxSearchDepth").get<int>();
        preset.useSafetyCheck   = entry.at("useSafetyCheck").get<bool>();
        preset.hasSurvivalMode  = entry.at("hasSurvivalMode").get<bool>();
        preset.randomMoveChance = entry.at("randomMoveChance").get<float>();
        preset.aggresiveness    = entry.at("aggressiveness").get<float>();

        const std::string name = entry.at("name").get<std::string>();
        table[name] = preset;
    }

    return table;
}
