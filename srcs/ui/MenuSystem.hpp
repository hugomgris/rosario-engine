#pragma once

#include <vector>
#include <iostream>
#include <raylib.h>
#include "UIQueue.hpp"
#include "ButtonConfigLoader.hpp"
#include "ButtonFactory.hpp"
#include "../ecs/Entity.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/FrameContext.hpp"


class Registry;
class InputSystem;
class ArenaGrid;

enum class MenuState {
    Start,
    Pause,
    GameOver
};

class MenuSystem {
    private:
        int _screenWidth;
        int _screenHeight;

        std::vector<Entity> _startButtonEntities;
        std::vector<Entity> _gameOverButtonEntities;

    public:
        MenuSystem(int sWidth, int sHeight);

        void setupStartButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table);
        
        void setupGameOverButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table);

        void clearStartButtons(Registry& registry);
        void clearGameOverButtons(Registry& registry);

        void buildStartMenuUI(Registry& registry, UIRenderQueue& queue);
        void buildGameOverUI(Registry& registry, UIRenderQueue& queue);

    private:
        void enqueueButtonRenderCommands(const ButtonComponent& button, UIRenderQueue& queue) const;
};