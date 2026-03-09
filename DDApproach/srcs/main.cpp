#include <iostream>
#include "ecs/Registry.hpp"
#include "core/GameController.hpp"
#include "data/GameConfig.hpp"
#include "data/ArenaConfig.hpp"
#include "data/AIConfig.hpp"

int main() {
    // Initialize the game configuration
    GameConfig gameConfig;
    ArenaConfig arenaConfig;
    AIConfig aiConfig;

    // Load configurations from JSON files
    gameConfig.load("configs/game_config.json");
    arenaConfig.load("configs/arena_default.json");
    aiConfig.load("configs/ai_config.json");

    // Create the entity registry
    Registry registry;

    // Initialize the game controller
    GameController gameController(&registry, gameConfig, arenaConfig, aiConfig);

    // Start the main game loop
    gameController.run();

    return 0;
}