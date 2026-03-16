#pragma once

#include <vector>
#include <iostream>
#include <raylib.h>
//#include "Button.hpp"
#include "UIQueue.hpp"
#include "../../incs/DataStructs.hpp"

class ParticleSystem;

enum class MenuState {
    Start,
    Pause,
    GameOver
};

class MenuSystem {
    private:
        std::string _placeholderStart = "S T A R T";
        std::string _placeholderGameOver = "G A M E  O V E R";
        MenuState   _state;

        int         _screenWidth;
        int         _screenHeight;

    public:
        MenuSystem(int sWidth, int sHeight);

        void setState(MenuState state, GameMode currentMode);

        void render(GameMode currentMode) const;
        void renderGameOver(GameMode currentMode) const;
};