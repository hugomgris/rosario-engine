#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class GameConfig {
public:
    GameConfig(const std::string& configFilePath);
    
    float getGameSpeed() const;
    int getDifficultyLevel() const;
    std::string getPlayerName() const;

private:
    void loadConfig(const std::string& configFilePath);
    
    float gameSpeed;
    int difficultyLevel;
    std::string playerName;
};

#endif // GAMECONFIG_HPP