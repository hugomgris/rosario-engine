#include "Factories.hpp"

Entity Factories::spawnPlayerSnake(Registry& registry,
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

Entity Factories::spawnFood(Registry& registry, Vec2 pos) {
	Entity e = registry.createEntity();
	registry.addComponent(e, FoodTag{});
	registry.addComponent(e, PositionComponent{ pos });
	
	return e;
}

void Factories::relocateFood(Registry& registry, Entity foodEntity,
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

void Factories::resetGame(Registry& registry,
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