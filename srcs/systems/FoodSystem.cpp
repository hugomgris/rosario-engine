#include "FoodSystem.hpp"

void FoodSystem::update(Registry& registry, const ArenaGrid* arena, int w, int h) {
	manageSnakeFoodCollision(registry, arena, w, h);
}

void FoodSystem::manageSnakeFoodCollision(Registry& registry, const ArenaGrid* arena, int w, int h) {
	for (auto entity : registry.view<SnakeComponent, CollisionResultComponent>()) {
		auto& snake = registry.getComponent<SnakeComponent>(entity);
		auto& result = registry.getComponent<CollisionResultComponent>(entity);

		if (result.result != CollisionType::Food) continue;

		result.result = CollisionType::None;
		snake.growing = true;

		for (auto entity : registry.view<FoodTag>()) {
			GameState::relocateFood(registry, entity, w, h, arena);
		}
	}
}