#include <iostream>
#include <algorithm>
#include <ctime>
#include <raylib.h>

#include "../incs/DataStructs.hpp"
#include "../incs/Colors.hpp"
#include "../incs/RaylibColors.hpp"
#include "ecs/Registry.hpp"
#include "components/PositionComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "systems/InputSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include "helpers/Factories.hpp"
#include "helpers/GameState.hpp"
#include "collision/CollisionRule.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "collision/CollisionEffects.hpp"

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    CollisionRuleTable ruleTable = CollisionRuleLoader::load("data/collisionRules.JSON");

    CollisionEffectDispatcher dispatcher;
    dispatcher.registerDefaults();

    // ECS core
    Registry registry;

    // Gameplay systems
    InputSystem     inputSystem;
    MovementSystem  movementSystem;
    CollisionSystem collisionSystem;
    RenderSystem    renderSystem;
    renderSystem.init(GRID_W, GRID_H);

    // initial world
    ArenaGrid arena(GRID_W, GRID_H);
    Entity playerSnake(0u), aiSnake(0u), food(0u);
    GameState::resetGame(registry, inputSystem, playerSnake, aiSnake, food, GRID_W, GRID_H, arena);
    RenderMode renderMode = RenderMode::MODE2D;

    while (true) {
        if (WindowShouldClose()) break;

        DrawFPS(SCREEN_W - 95, 10);

        const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();

        bool playerDied = false;

        // fresh context
        CollisionEffects::EffectContext ctx {
            &arena,         // arena
            GRID_W,         // gridWidth
            GRID_H,         // gridHeight
            &playerDied     // playerDied — KillSnake writes here
        };

        // update phase
        inputSystem.update(registry);
        movementSystem.update(registry, dt);
        collisionSystem.update(registry, ruleTable, dispatcher, ctx);

        if (playerDied) {
            std::cout << "PLAYER DIED" << std::endl;
            break;
        }

        // render phase
        renderSystem.render(registry, renderMode, dt, &arena);
    }

    CloseWindow();
    return 0;
}