#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/MovementComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/RenderComponent.hpp"
#include "../components/ScoreComponent.hpp"
#include "../components/AIComponent.hpp"
#include "../components/FoodTag.hpp"
#include "../components/SolidTag.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../systems/InputSystem.hpp"
#include "../AI/AIPresetLoader.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/Colors.hpp"
#include "../../incs/RaylibColors.hpp"

class Factories {
	public:
		static Entity spawnPlayerSnake(Registry& registry,
										InputSystem& inputSystem,
										Vec2 startPos,
										int initialLength,
										BaseColor color,
										PlayerSlot slot);

		static Entity spawnAISnake(Registry& registry,
								Vec2 startPos,
								int  initialLength,
								BaseColor color,
								const std::string& presetName,
								const AIPresetLoader::PresetTable& presets);

		static Entity spawnFood(Registry& registry, Vec2 pos);
};