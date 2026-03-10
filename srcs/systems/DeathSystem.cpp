#include "DeathSystem.hpp"

void DeathSystem::update(Registry& registry, Entity playerSnake, bool* playerDied) {
	for (auto& entity : registry.view<CollisionResultComponent>()) {
		auto& result = registry.getComponent<CollisionResultComponent>(entity);
		
		if (result.result != CollisionType::Self &&
			result.result != CollisionType::Wall && 
			result.result != CollisionType::Snake) {
				continue;
			}

		if (entity == playerSnake) {
			result.result = CollisionType::None;
			*playerDied = true;
		}
	}
}