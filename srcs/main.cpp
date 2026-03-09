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

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

// entity factories
static Entity spawnPlayerSnake(Registry& registry,
								InputSystem& inputSystem,
								Vec2 startPos,
								int initialLength,
								BaseColor color,
								PlayerSlot slot) {
	Entity e = registry.createEntity();
	SnakeComponent snake;

	for (int i = 0; i < initialLength; ++i) {
		snake.segments.push_back({ { startPos.x - i, startPos.y }, BeadType::None });
	}

	registry.addComponent(e, snake);
	registry.addComponent(e, PositionComponent{ startPos });
	registry.addComponent(e, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
	registry.addComponent(e, InputComponent{});
	registry.addComponent(e, RenderComponent{ color });
	registry.addComponent(e, CollisionResultComponent{});
	inputSystem.assignSlot(e, slot);
	return e;
} 

// Todo: spawn ai snake

static Entity spawnFood(Registry& registry, Vec2 pos) {
	Entity e = registry.createEntity();
	registry.addComponent(e, FoodTag{});
	registry.addComponent(e, PositionComponent{ pos });
	
	return e;
}

static void relocateFood(Registry& registry, Entity foodEntity,
						int gridWidth, int gridHeight,
						const ArenaGrid* arena = nullptr) {
	std::vector<Vec2> snakeOccupied;
	for (auto e : registry.view<SnakeComponent>())
		for (const auto& seg : registry.getComponent<SnakeComponent>(e).segments)
			snakeOccupied.push_back(seg.position);

	auto isSnakeOccupied = [&](Vec2 p) {
		for (const auto& o : snakeOccupied)
			if (o.x == p.x && o.y == p.y) return true;
		return false;
	};

	if (arena) {
		auto cells = arena->getAvailableCells();
		for (int tries = 0; tries < static_cast<int>(cells.size()); ++tries) {
			Vec2 c = cells[static_cast<size_t>(std::rand()) % cells.size()];
			if (!isSnakeOccupied(c)) {
				registry.getComponent<PositionComponent>(foodEntity).position = c;
				return;
			}
		}
		return;
	}

	std::vector<Vec2> solidOccupied;
	for (auto e : registry.view<SolidTag, PositionComponent>())
		solidOccupied.push_back(registry.getComponent<PositionComponent>(e).position);

	auto isOccupied = [&](Vec2 p) {
		for (const auto& o : solidOccupied)
			if (o.x == p.x && o.y == p.y) return true;
		return isSnakeOccupied(p);
	};

	for (int attempts = 0; attempts < gridWidth * gridHeight; ++attempts) {
		Vec2 candidate = { std::rand() % gridWidth, std::rand() % gridHeight };
		if (!isOccupied(candidate)) {
			registry.getComponent<PositionComponent>(foodEntity).position = candidate;
			return;
		}
	}
}

static void resetGame(Registry& registry,
						InputSystem& inputSystem,
						Entity& playerSnake, Entity& aiSnake, Entity& food,
						int gridWidth, int gridHeight,
						ArenaGrid& arena) {
	// wipe the registry and re-create
	registry = Registry{};

	const BaseColor snakeAColor = { 135, 206, 250, 255 }; // TODO: un-hardcode this
	const BaseColor snakeAiColor = { 46, 179, 113, 255 };

	playerSnake = spawnPlayerSnake(registry, inputSystem,
									{ gridWidth / 2, gridHeight / 2},
									4, snakeAColor, PlayerSlot::A);

	/*
	aiSnake     = spawnAISnake(registry,
                                { gridWidth / 2 + 6,  gridHeight / 2 },
                                4, snakeAIColor, AIComponent::medium());
	*/

	food = spawnFood(registry, { gridWidth / 4, gridHeight / 4 });

	arena.clearArena();

	/* WallPreset preset = ArenaPresets::getRandomPreset();
	arena.transformArenaWithPreset(preset);
	arena.beginSpawn(1.2f); */ //TO DO
}

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
	resetGame(registry, inputSystem, playerSnake, aiSnake, food, GRID_W, GRID_H, arena);
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
					relocateFood(registry, food, GRID_W, GRID_H, &arena);
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
