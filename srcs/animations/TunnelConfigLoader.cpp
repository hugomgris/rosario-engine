#include "TunnelConfigLoader.hpp"
#include "../../incs/third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

TunnelConfigLoader::PresetTable TunnelConfigLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("TunnelConfigLoader: cannot open: " + path);

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("TunnelConfigLoader: parse error: " + std::string(e.what()));
    }

    PresetTable table;

    for (const auto& entry : data.at("presets")) {
        TunnelConfig cfg;

        cfg.borderThickness = entry.at("borderThickness").get<int>();
        cfg.contentInset    = entry.at("contentInset").get<int>();
        cfg.spawnInterval   = entry.at("spawnInterval").get<float>();
        cfg.animationSpeed  = entry.at("animationSpeed").get<float>();
        cfg.maxLines        = entry.at("maxLines").get<int>();
        cfg.lineThickness   = entry.at("lineThickness").get<float>();

        const auto& col = entry.at("lineColor");
        cfg.lineColor = Color{
            static_cast<unsigned char>(col[0].get<int>()),
            static_cast<unsigned char>(col[1].get<int>()),
            static_cast<unsigned char>(col[2].get<int>()),
            static_cast<unsigned char>(col[3].get<int>())
        };

        table[entry.at("name").get<std::string>()] = cfg;
    }

    return table;
}
