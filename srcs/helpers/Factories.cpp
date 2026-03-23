#include "Factories.hpp"
#include <stdexcept>

Entity Factories::spawnPlayerSnake(Registry& registry,
								InputSystem& inputSystem,
								Vec2 startPos,
								int initialLength,
								BaseColor color,
								PlayerSlot slot) {
	Entity e = registry.createEntity();
	SnakeComponent snake;

	Direction direction = (registry.view<SnakeComponent>().empty()) ? Direction::RIGHT : Direction::LEFT;

	for (int i = 0; i < initialLength; ++i) {
        Vec2 segPos;
        switch (direction) {
            case Direction::RIGHT: segPos = { startPos.x - i, startPos.y }; break;
            case Direction::LEFT:  segPos = { startPos.x + i, startPos.y }; break;
            case Direction::DOWN:  segPos = { startPos.x, startPos.y - i }; break;
            case Direction::UP:    segPos = { startPos.x, startPos.y + i }; break;
        }
        snake.segments.push_back({ segPos, BeadType::None });
    }

	registry.addComponent(e, RenderComponent{ color });
	registry.addComponent(e, ScoreComponent{});
	snake.slot = slot;
	registry.addComponent(e, snake);
	registry.addComponent(e, PositionComponent{ startPos });
	registry.addComponent(e, MovementComponent{ direction, 0.0f, 0.1f });
	registry.addComponent(e, InputComponent{});
	inputSystem.assignSlot(e, slot);
	return e;
} 

Entity Factories::spawnAISnake(Registry& registry,
							Vec2 startPos,
							int  initialLength,
							BaseColor color,
							const std::string& presetName,
							const AIPresetLoader::PresetTable& presets) {
	auto it = presets.find(presetName);
	if (it == presets.end())
		throw std::runtime_error("Factories::spawnAISnake: unknown preset: " + presetName);

	Entity e = registry.createEntity();
	SnakeComponent snake;
	for (int i = 0; i < initialLength; ++i)
		snake.segments.push_back({ { startPos.x - i, startPos.y }, BeadType::None });
	registry.addComponent(e, snake);
	registry.addComponent(e, PositionComponent{ startPos });
	registry.addComponent(e, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
	registry.addComponent(e, it->second);
	registry.addComponent(e, RenderComponent{ color });
	return e;
}

Entity Factories::spawnFood(Registry& registry, Vec2 pos) {
	Entity e = registry.createEntity();
	registry.addComponent(e, FoodTag{});
	registry.addComponent(e, PositionComponent{ pos });
	
	return e;
}

Entity Factories::spawnPixelText(Registry& registry,
							 const PixelTextComponent& pixelText,
							 bool withLayout) {
	Entity e = registry.createEntity();
	registry.addComponent(e, pixelText);
	if (withLayout) {
		registry.addComponent(e, PixelTextLayoutComponent{});
	}
	return e;
}