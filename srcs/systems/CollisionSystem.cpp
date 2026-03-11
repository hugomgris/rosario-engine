#include "CollisionSystem.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/SolidTag.hpp"
#include "../components/FoodTag.hpp"

void CollisionSystem::resolveCollision(const std::string& subjectType,
									const std::string& objectType,
									Entity subject,
									Entity object,
									Registry& registry,
									const CollisionRuleTable& table,
									const CollisionEffectDispatcher& dispatcher,
									const CollisionEffects::EffectContext& ctx) {
	const CollisionRule* rule = table.find(subjectType, objectType);
	if (!rule) return; // no rule defined for this pair, so ignore silently
	
	// DEBUG
	std::cout << "COLLISION:" << rule->subject << " - " << rule->object << std::endl;

	for (const auto& effectName : rule->effects)
		dispatcher.execute(effectName, registry, subject, object, ctx);
}

void CollisionSystem::checkWallCollisions(Registry& registry,
										const CollisionRuleTable& table,
										const CollisionEffectDispatcher& dispatcher,
										const CollisionEffects::EffectContext& ctx) {
	if (ctx.arena) {
		for (auto entity : registry.view<SnakeComponent, PositionComponent>()) {
			const Vec2 head = registry.getComponent<PositionComponent>(entity).position;
			if (!ctx.arena->isWalkable(head.x, head.y))
				resolveCollision("Snake", "Wall", entity, entity, registry, table, dispatcher, ctx);
		}
		return;
	}

	// legacy fallback
	std::vector<Vec2> solidPositions;
	for (auto solid : registry.view<SolidTag, PositionComponent>())
		solidPositions.push_back(registry.getComponent<PositionComponent>(solid).position);

	for (auto entity : registry.view<SnakeComponent, PositionComponent>()) {
		const Vec2 head = registry.getComponent<PositionComponent>(entity).position;
		for (const auto& solidPos : solidPositions) {
			if (head.x == solidPos.x && head.y == solidPos.y) {
				resolveCollision("Snake", "Wall", entity, entity, registry, table, dispatcher, ctx);
				break;
			}
		}
	}
}

void CollisionSystem::checkSelfCollisions(Registry& registry,
										const CollisionRuleTable& table,
										const CollisionEffectDispatcher& dispatcher,
										const CollisionEffects::EffectContext& ctx) {
	for (auto entity : registry.view<SnakeComponent>()) {
		const auto& snake = registry.getComponent<SnakeComponent>(entity);
		if (snake.segments.size() < 2) continue;

		const Vec2 head = snake.segments.front().position;
		for (size_t i = 1; i < snake.segments.size(); ++i) {
			if (head.x == snake.segments[i].position.x &&
				head.y == snake.segments[i].position.y) {
				resolveCollision("Snake", "Self", entity, entity, registry, table, dispatcher, ctx);
				break;
			}
		}
	}
}

void CollisionSystem::checkSnakeCollisions(Registry& registry,
										const CollisionRuleTable& table,
										const CollisionEffectDispatcher& dispatcher,
										const CollisionEffects::EffectContext& ctx) {
	auto snakes = registry.view<SnakeComponent, PositionComponent>();

	for (auto entityA : snakes) {
		const Vec2 headA = registry.getComponent<PositionComponent>(entityA).position;

		for (auto entityB : snakes) {
			if (entityA == entityB) continue;

			const auto& snakeB = registry.getComponent<SnakeComponent>(entityB);
			for (const auto& seg : snakeB.segments) {
				if (headA.x == seg.position.x && headA.y == seg.position.y) {
					resolveCollision("Snake", "Snake", entityA, entityB, registry, table, dispatcher, ctx);
					break;
				}
			}
		}
	}
}

void CollisionSystem::checkFoodCollisions(Registry& registry,
										const CollisionRuleTable& table,
										const CollisionEffectDispatcher& dispatcher,
										const CollisionEffects::EffectContext& ctx) {
	std::vector<std::pair<Entity, Vec2>> foodEntities;
	for (auto food : registry.view<FoodTag, PositionComponent>())
		foodEntities.push_back({ food, registry.getComponent<PositionComponent>(food).position });

	for (auto entity : registry.view<SnakeComponent, PositionComponent>()) {
		const Vec2 head = registry.getComponent<PositionComponent>(entity).position;

		for (const auto& [foodEntity, foodPos] : foodEntities) {
			if (head.x == foodPos.x && head.y == foodPos.y) {
				resolveCollision("Snake", "Food", entity, foodEntity, registry, table, dispatcher, ctx);
				break;
			}
		}
	}
}

void CollisionSystem::update(Registry& registry,
							const CollisionRuleTable& table,
							const CollisionEffectDispatcher& dispatcher,
							const CollisionEffects::EffectContext& ctx) {
	checkWallCollisions (registry, table, dispatcher, ctx);
	checkSelfCollisions (registry, table, dispatcher, ctx);
	checkSnakeCollisions(registry, table, dispatcher, ctx);
	checkFoodCollisions (registry, table, dispatcher, ctx);
}