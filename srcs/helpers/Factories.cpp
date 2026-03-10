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
	registry.addComponent(e, ScoreComponent{});
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