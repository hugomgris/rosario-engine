#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../systems/InputSystem.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../AI/AIPresetLoader.hpp"
#include "../../incs/Colors.hpp"
#include "Factories.hpp"

class GameState {
	public:
		static void resetGame(Registry& registry,
								InputSystem& inputSystem,
								Entity& playerSnake, Entity& aiSnake, Entity& food,
								int gridWidth, int gridHeight,
								ArenaGrid& arena,
								const AIPresetLoader::PresetTable& AIPresets);

		static void relocateFood(Registry& registry, Entity foodEntity,
								int gridWidth, int gridHeight,
								const ArenaGrid* arena = nullptr);
};