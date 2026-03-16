#include "GameManager.hpp"

void GameManager::resetGame(Registry& registry,
						InputSystem& inputSystem,
						Entity& playerSnake, Entity& secondSnake, Entity& food,
						int gridWidth, int gridHeight,
						ArenaGrid& arena,
						const AIPresetLoader::PresetTable& AIPresets, GameMode mode) {
	// wipe the registry and re-create
	registry = Registry{};
	inputSystem.clearSlots();

	SetColors colorSet;

	Vec2 firstSnakePos = { gridWidth / 2, (gridHeight / 3) };
	Vec2 secondSnakePos = { gridWidth / 2, (gridHeight / 3) * 2 };

	playerSnake = Factories::spawnPlayerSnake(registry, inputSystem,
									firstSnakePos,
									4, colorSet.snakeAColor, PlayerSlot::A);

	if (mode == GameMode::VSAI) {
		secondSnake = Factories::spawnAISnake(registry,
								secondSnakePos,
								4, colorSet.snakeAIColor, "medium", AIPresets);
	} else if (mode == GameMode::MULTI) {
		secondSnake = Factories::spawnPlayerSnake(registry, inputSystem,
									secondSnakePos,
									4, colorSet.snakeBColor, PlayerSlot::B);
	}
	
	food = Factories::spawnFood(registry, { gridWidth / 4, gridHeight / 4 });

	arena.clearArena();

	/* WallPreset preset = ArenaPresets::getRandomPreset();
	arena.transformArenaWithPreset(preset);
	arena.beginSpawn(1.2f); */ //TO DO
}

void GameManager::relocateFood(Registry& registry, Entity foodEntity,
						int gridWidth, int gridHeight,
						const ArenaGrid* arena) {
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