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
	CollisionSystem	collisionSystem;
	RenderSystem		renderSystem;
	renderSystem.init(GRID_W, GRID_H);

	// initial world
	ArenaGrid arena(GRID_W, GRID_H);
	Entity playerSnake(0u), aiSnake(0u), food(0u); // TODO: ai snake
	Factories::resetGame(registry, inputSystem, playerSnake, aiSnake, food, GRID_W, GRID_H, arena);
	RenderMode renderMode = RenderMode::MODE2D;

	// state machine
	//TODO

	while (true) {
		if (WindowShouldClose()) break;

		DrawFPS(SCREEN_W - 95, 10);

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f); // TODO: why this?

		if (IsKeyPressed(KEY_F)) ToggleFullscreen(); // TODO: ?

		inputSystem.update(registry);
		movementSystem.update(registry, dt);
		collisionSystem.update(registry, &arena);

		bool playerDied = false;
		for (auto entity : registry.view<CollisionResultComponent>()) {
			auto& result = registry.getComponent<CollisionResultComponent>(entity);
			switch (result.result) {
				case CollisionType::Food:
					Factories::relocateFood(registry, food, GRID_W, GRID_H, &arena);
					result.result = CollisionType::None;
					break;

				case CollisionType::Wall:
				case CollisionType::Self:
				case CollisionType::Snake:
					if (entity == playerSnake) playerDied = true;
					result.result = CollisionType::None;
					break;

				case CollisionType::None:
					break;
			}
		}

		if (playerDied) {
			std::cout << "PLAYER DIED" << std::endl; // TODO: menu mangling
			break;
		}

		// render
		renderSystem.render(registry, renderMode, dt); // TODO: add arena
	}

	CloseWindow();
	return 0;
}
