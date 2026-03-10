#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/MovementComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/RenderComponent.hpp"
#include "../components/CollisionResultComponent.hpp"
#include "../components/FoodTag.hpp"
#include "../components/SolidTag.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../systems/InputSystem.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/Colors.hpp"
#include "../../incs/RaylibColors.hpp"

class Factories {
	public:
		// entity factories
		static Entity spawnPlayerSnake(Registry& registry,
										InputSystem& inputSystem,
										Vec2 startPos,
										int initialLength,
										BaseColor color,
										PlayerSlot slot);

		// Todo: spawn ai snake

		static Entity spawnFood(Registry& registry, Vec2 pos);

		static void relocateFood(Registry& registry, Entity foodEntity,
								int gridWidth, int gridHeight,
								const ArenaGrid* arena = nullptr);

		static void resetGame(Registry& registry,
								InputSystem& inputSystem,
								Entity& playerSnake, Entity& aiSnake, Entity& food,
								int gridWidth, int gridHeight,
								ArenaGrid& arena);
};