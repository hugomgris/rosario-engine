#pragma once

#include <unordered_map>
#include "../ecs/Registry.hpp"
#include "../../incs/DataStructs.hpp"

enum class PlayerSlot {
	A,
	B,
};

class InputSystem {
	private:
		void pollPlayerA(Registry& registry, Entity entity);
		void pollPlayerB(Registry& registry, Entity entity);

		std::unordered_map<Entity::ID, PlayerSlot> slotMap;

	public:
		void update(Registry& registry);
		void assignSlot(Entity entity, PlayerSlot slot);
};