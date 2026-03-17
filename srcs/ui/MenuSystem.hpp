#pragma once

#include <vector>
#include <iostream>
#include <raylib.h>
#include "Button.hpp"
#include "UIQueue.hpp"
#include "ButtonConfigLoader.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/FrameContext.hpp"


class ParticleSystem;
class Registry;
class InputSystem;
class Entity;
class ArenaGrid;
class AIPresetLoader;

enum class MenuState {
    Start,
    Pause,
    GameOver
};

// Context struct to hold all game resources needed by button actions
struct MenuContext {
	GameState* state = nullptr;
	GameMode* mode = nullptr;
	Registry* registry = nullptr;
	InputSystem* inputSystem = nullptr;
	Entity* playerSnake = nullptr;
	Entity* secondSnake = nullptr;
	Entity* food = nullptr;
	int gridWidth = 0;
	int gridHeight = 0;
	ArenaGrid* arena = nullptr;
	void* AIPresets = nullptr; // AIPresetLoader::PresetTable*
};

class MenuSystem {
    private:
        std::string _placeholderStart = "S T A R T";
        std::string _placeholderGameOver = "G A M E  O V E R";
        MenuState   _state;

        std::vector<Button> _startButtons;
        std::vector<Button> _gameOverButtons;

        int         _screenWidth;
        int         _screenHeight;

        void setupButtons(const ButtonConfigLoader::MenuButtonTable& table, std::vector<Button>& target, const MenuContext& ctx);
        void enqueueButtons(const std::vector<Button>& buttons, UIRenderQueue& queue) const;

        // function hook-up method
        std::function<void()> resolveAction(const std::string& id, const MenuContext& ctx);
        //void onStartClickButton(GameState* state);

    public:
        MenuSystem(int sWidth, int sHeight);

        void setupStartButtons(const ButtonConfigLoader::MenuButtonTable& table, const MenuContext& ctx);
        void setupGameOverButtons(const ButtonConfigLoader::MenuButtonTable& table, const MenuContext& ctx);

        void setState(MenuState state, GameMode currentMode);

        void buildStartMenuUI(FrameContext &ctx, UIRenderQueue& queue);
        void buildGameOverUI(FrameContext &ctx, UIRenderQueue& queue);

        void update(GameState& state);
};