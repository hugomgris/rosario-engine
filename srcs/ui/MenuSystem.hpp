#pragma once

#include <vector>
#include <raylib.h>
#include "Button.hpp"
#include "TextSystem.hpp"
#include "../../incs/DataStructs.hpp"

class ParticleSystem;

enum class MenuState {
    Start,
    GameOver
};

class MenuSystem {
private:
    std::vector<Button> _buttons;
    MenuState           _menuState          = MenuState::Start;
    int                 _selectedIndex      = -1;

    int                 _screenWidth        = 1920;
    int                 _screenHeight       = 1080;

    int                 _logoTrailCounter   = 0;

    void initButtons(GameMode currentMode);
    void clearButtons();

    void selectNext();
    void selectPrev();

    // navigation helpers
    enum class NavAction { None, Up, Down, Confirm, Cancel };
    NavAction pollNavigation() const;

public:
    MenuSystem(int screenWidth, int screenHeight);

    void setState(MenuState state, GameMode currentMode);

    // Returns the action that main should execute this frame (None if nothing changed)
    MenuAction update(float dt, GameMode currentMode, ParticleSystem& particles);

    void render(TextSystem& text, GameMode currentMode) const;
    void renderGameOver(TextSystem& text, GameMode currentMode) const;
};
