#pragma once

#include "../../incs/DataStructs.hpp"
#include "../ecs/Registry.hpp"

class MovementSystem {
	private:
		void processInput(Registry& registry);
		void advanceSnake(Registry& registry, float deltaTime);

		Vec2 directionToVec2(Direction dir) const;
	
	public:
		void update(Registry& registry, float deltaTime);
};