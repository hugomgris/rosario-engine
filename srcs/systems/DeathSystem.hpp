#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/CollisionResultComponent.hpp"

class DeathSystem {
	public:
		void update(Registry& registry, Entity playerSnake, bool* playerDied);
};