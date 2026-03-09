#ifndef GAMECONTROLLER_HPP
#define GAMECONTROLLER_HPP

#include "ecs/Registry.hpp"
#include "data/GameConfig.hpp"
#include "data/ArenaConfig.hpp"
#include "data/AIConfig.hpp"
#include "arena/Arena.hpp"
#include "graphics/Renderer.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/AISystem.hpp"
#include "systems/InputSystem.hpp"

class GameController {
public:
    GameController(const GameConfig& gameConfig, const ArenaConfig& arenaConfig, const AIConfig& aiConfig);
    void initialize();
    void run();
    void update(float deltaTime);
    void render();
    void cleanup();

private:
    Registry registry;
    GameConfig gameConfig;
    ArenaConfig arenaConfig;
    AIConfig aiConfig;
    Arena arena;
    Renderer renderer;

    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    RenderSystem renderSystem;
    AISystem aiSystem;
    InputSystem inputSystem;

    bool isRunning;
};

#endif // GAMECONTROLLER_HPP