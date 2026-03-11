#pragma once

#include "../../incs/DataStructs.hpp"
#include "../ecs/Registry.hpp"
#include "../components/MovementComponent.hpp"
#include "../components/InputComponent.hpp"

class MovementSystem {
	private:
		void processInput(Registry& registry, Direction lastDirection, MovementComponent& move, InputComponent& input);
		void advanceSnake(Registry& registry, float deltaTime);

		Vec2 directionToVec2(Direction dir) const;
	
	public:
		void update(Registry& registry, float deltaTime);
};