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
//#include "components/AIComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/CollisionResultComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "systems/InputSystem.hpp"
//#include "systems/AISystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/FoodSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/RenderSystem.hpp"
//#include "systems/ParticleSystem.hpp"
//#include "systems/TextSystem.hpp"
//#include "systems/AnimationSystem.hpp"
//#include "systems/MenuSystem.hpp"
//#include "systems/PostProcessingSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
//#include "core/GameState.hpp"
#include "helpers/Factories.hpp"
#include "helpers/GameState.hpp"

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

int main() {
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	// ECS core
	Registry registry;

	// Gameplay systems
	InputSystem			inputSystem;
	//AISystem			aiSystem(GRID_W, GRID_H);
	MovementSystem		movementSystem;
	CollisionSystem		collisionSystem;
	FoodSystem			foodSystem;
	DeathSystem			deathSystem;
	RenderSystem		renderSystem;
	renderSystem.init(GRID_W, GRID_H);

	// initial world
	ArenaGrid arena(GRID_W, GRID_H);
	Entity playerSnake(0u), aiSnake(0u), food(0u); // TODO: ai snake
	GameState::resetGame(registry, inputSystem, playerSnake, aiSnake, food, GRID_W, GRID_H, arena);
	RenderMode renderMode = RenderMode::MODE2D;

	// state machine
	//TODO

	while (true) {
		if (WindowShouldClose()) break;

		DrawFPS(SCREEN_W - 95, 10);

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f); // TODO: why this?

		if (IsKeyPressed(KEY_F)) ToggleFullscreen(); // TODO: ?

		bool playerDied = false;

		// update and management system phase
		inputSystem.update(registry);
		movementSystem.update(registry, dt);
		collisionSystem.update(registry, &arena);
		foodSystem.update(registry, &arena, GRID_W, GRID_H);
		deathSystem.update(registry, playerSnake, &playerDied);

		if (playerDied) {
			std::cout << "PLAYER DIED" << std::endl; // TODO: menu mangling
			break;
		}

		// render phase
		renderSystem.render(registry, renderMode, dt, &arena);
	}

	CloseWindow();
	return 0;
}
