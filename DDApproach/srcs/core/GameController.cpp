#include "GameController.hpp"
#include "InputManager.hpp"
#include "Arena.hpp"
#include "GameConfig.hpp"
#include "AIConfig.hpp"
#include "Registry.hpp"
#include "Entity.hpp"
#include "Renderer.hpp"
#include "MovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"
#include "AISystem.hpp"
#include "AnimationSystem.hpp"
#include "InputSystem.hpp"
#include <iostream>
#include <memory>

class GameController {
public:
    GameController();
    void initialize();
    void run();
    void update();
    void render();
    void cleanup();

private:
    std::unique_ptr<Registry> registry;
    std::unique_ptr<Arena> arena;
    std::unique_ptr<GameConfig> gameConfig;
    std::unique_ptr<AIConfig> aiConfig;

    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    RenderSystem renderSystem;
    AISystem aiSystem;
    AnimationSystem animationSystem;
    InputSystem inputSystem;

    bool isRunning;
};

GameController::GameController() 
    : registry(std::make_unique<Registry>()), 
      arena(std::make_unique<Arena>()), 
      gameConfig(std::make_unique<GameConfig>()), 
      aiConfig(std::make_unique<AIConfig>()), 
      isRunning(true) {}

void GameController::initialize() {
    // Load configurations
    gameConfig->load("configs/game_config.json");
    aiConfig->load("configs/ai_config.json");
    
    // Initialize systems
    movementSystem.initialize(registry.get());
    collisionSystem.initialize(registry.get());
    renderSystem.initialize(registry.get());
    aiSystem.initialize(registry.get());
    animationSystem.initialize(registry.get());
    inputSystem.initialize();
    
    // Setup arena
    arena->initialize();
}

void GameController::run() {
    while (isRunning) {
        update();
        render();
    }
    cleanup();
}

void GameController::update() {
    inputSystem.processInput();
    movementSystem.update();
    collisionSystem.update();
    aiSystem.update();
}

void GameController::render() {
    renderSystem.render();
}

void GameController::cleanup() {
    // Cleanup resources if necessary
}

int main() {
    GameController gameController;
    gameController.initialize();
    gameController.run();
    return 0;
}