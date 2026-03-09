#ifndef ARENACONFIG_HPP
#define ARENACONFIG_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class ArenaConfig {
public:
    struct WallConfig {
        std::string type;
        int thickness;
        std::string color;
    };

    struct ArenaDimensions {
        int width;
        int height;
    };

    ArenaConfig(const std::string& configFilePath);
    void loadConfig();
    
    ArenaDimensions getDimensions() const;
    const std::vector<WallConfig>& getWallConfigs() const;

private:
    std::string configFilePath;
    ArenaDimensions dimensions;
    std::vector<WallConfig> wallConfigs;

    void parseConfig(const nlohmann::json& json);
};

#endif // ARENACONFIG_HPP