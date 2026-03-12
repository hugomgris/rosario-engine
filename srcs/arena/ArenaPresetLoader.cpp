#include "ArenaPresetLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

static WallPreset parsePreset(const std::string& s) {
    if (s == "InterLock1")   return WallPreset::InterLock1;
    if (s == "Spiral1")      return WallPreset::Spiral1;
    if (s == "Columns1")     return WallPreset::Columns1;
    if (s == "Columns2")     return WallPreset::Columns2;
    if (s == "Cross")        return WallPreset::Cross;
    if (s == "Checkerboard") return WallPreset::Checkerboard;
    if (s == "Maze")         return WallPreset::Maze;
    if (s == "Diamond")      return WallPreset::Diamond;
    if (s == "Tunnels")      return WallPreset::Tunnels;
    if (s == "FourRooms")    return WallPreset::FourRooms;
    throw std::runtime_error("ArenaPresetLoader: unknown preset: " + s);
}

std::vector<WallPreset> ArenaPresetLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("ArenaPresetLoader: cannot open: " + path);

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("ArenaPresetLoader: parse error: " + std::string(e.what()));
    }

    std::vector<WallPreset> presets;
    for (const auto& entry : data.at("presets"))
        presets.push_back(parsePreset(entry.at("name").get<std::string>()));

    return presets;
}
