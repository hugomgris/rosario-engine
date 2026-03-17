#pragma once

#include <unordered_map>
#include <string>
#include "ButtonConfig.hpp"

class ButtonConfigLoader {
    public:
        using MenuButtonTable = std::unordered_map<std::string, ButtonConfig>;

        struct ButtonTable {
            MenuButtonTable start;
            MenuButtonTable gameOver;
        };
        
        static ButtonTable load(const std::string& path);
};