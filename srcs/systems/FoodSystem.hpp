#pragma once

#include "../ecs/Registry.hpp"
#include "../components/CollisionResultComponent.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/FoodTag.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../helpers/GameState.hpp"

class FoodSystem {
	public:
		void update(Registry& registry, const ArenaGrid* arena, int w, int h);

	private:
		void manageSnakeFoodCollision(Registry& registry, const ArenaGrid* arena, int w, int h);
		//void setSnakeForGrowth(Registry& registry);
		//void triggerFoodRelocation(Registry& registry, const ArenaGrid* arena, int w, int h);
};