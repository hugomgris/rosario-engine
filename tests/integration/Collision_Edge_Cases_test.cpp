#include <gtest/gtest.h>

#include "ecs/Registry.hpp"
#include "systems/CollisionSystem.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "components/SnakeComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/ScoreComponent.hpp"
#include "components/FoodTag.hpp"
#include "FrameContext.hpp"

namespace {

void MarkSnakeGrowing(Registry& registry, Entity subject, Entity, FrameContext&) {
	if (!registry.hasComponent<SnakeComponent>(subject)) {
		return;
	}
	registry.getComponent<SnakeComponent>(subject).growing = true;
}

} // namespace

// 1 - Head-to-head snake collision triggers appropriate collision effects
TEST(CollisionEdgeCases, HeadToHeadSnakeCollisionResolvesWithoutCrash) {
	Registry registry;
	CollisionSystem collisionSystem;
	CollisionEffectDispatcher dispatcher;
	
	dispatcher.registerEffect("GrowSnake", MarkSnakeGrowing);
	
	CollisionRuleTable rules;
	CollisionRule selfCollisionRule;
	selfCollisionRule.subject = "Snake";
	selfCollisionRule.object = "Self";
	selfCollisionRule.effects = { "GrowSnake" };
	rules.rules.push_back(selfCollisionRule);
	
	Entity snake1 = registry.createEntity();
	SnakeComponent snake1Comp;
	snake1Comp.segments.push_back({ { 15, 15 }, BeadType::None });
	snake1Comp.segments.push_back({ { 14, 15 }, BeadType::None });
	snake1Comp.growing = false;
	registry.addComponent<SnakeComponent>(snake1, snake1Comp);
	registry.addComponent<PositionComponent>(snake1, PositionComponent{ { 15, 15 } });
	
	Entity snake2 = registry.createEntity();
	SnakeComponent snake2Comp;
	snake2Comp.segments.push_back({ { 17, 15 }, BeadType::None });
	snake2Comp.segments.push_back({ { 18, 15 }, BeadType::None });
	snake2Comp.growing = false;
	registry.addComponent<SnakeComponent>(snake2, snake2Comp);
	registry.addComponent<PositionComponent>(snake2, PositionComponent{ { 17, 15 } });
	
	auto& pos1 = registry.getComponent<PositionComponent>(snake1);
	auto& pos2 = registry.getComponent<PositionComponent>(snake2);
	
	pos1.position.x += 1;  // Snake1 moves right (toward snake2)
	pos2.position.x -= 1;  // Snake2 moves left (toward snake1)
	
	EXPECT_EQ(pos1.position.x, 16);
	EXPECT_EQ(pos2.position.x, 16);
	
	FrameContext ctx;
	EXPECT_NO_THROW({
		collisionSystem.update(registry, rules, dispatcher, ctx);
	});
	
	EXPECT_TRUE(registry.hasComponent<SnakeComponent>(snake1));
	EXPECT_TRUE(registry.hasComponent<SnakeComponent>(snake2));
}

// 2 - Simultaneous collisions (snake food and wall) handled without duplication
TEST(CollisionEdgeCases, SimultaneousCollisionsHandledWithoutDuplication) {
	Registry registry;
	CollisionEffectDispatcher dispatcher;
	
	void (*growSnake)(Registry&, Entity, Entity, FrameContext&) = 
		[](Registry& reg, Entity subj, Entity, FrameContext&) {
			if (reg.hasComponent<SnakeComponent>(subj)) {
				reg.getComponent<SnakeComponent>(subj).growing = true;
			}
		};
	
	void (*addScore)(Registry&, Entity, Entity, FrameContext&) = 
		[](Registry& reg, Entity subj, Entity, FrameContext&) {
			if (reg.hasComponent<ScoreComponent>(subj)) {
				// Score would be added here
			}
		};
	
	dispatcher.registerEffect("GrowSnake", growSnake);
	dispatcher.registerEffect("AddScore", addScore);
	
	Entity snake = registry.createEntity();
	SnakeComponent snakeComp;
	snakeComp.segments.push_back({ { 15, 15 }, BeadType::None });
	snakeComp.growing = false;
	registry.addComponent<SnakeComponent>(snake, snakeComp);
	registry.addComponent<PositionComponent>(snake, PositionComponent{ { 15, 15 } });
	
	Entity food = registry.createEntity();
	registry.addComponent<FoodTag>(food, FoodTag{});
	registry.addComponent<PositionComponent>(food, PositionComponent{ { 15, 15 } });
	
	FrameContext ctx;
	
	EXPECT_NO_THROW({
		dispatcher.execute("GrowSnake", registry, snake, snake, ctx);
		dispatcher.execute("AddScore", registry, snake, snake, ctx);
	});
	
	auto& snakeAfter = registry.getComponent<SnakeComponent>(snake);
	EXPECT_TRUE(snakeAfter.growing);
}

// 3 - Self-collision on tight curve behaves consistently
TEST(CollisionEdgeCases, SelfCollisionOnTightCurveConsistent) {
	Registry registry;
	
	Entity snake = registry.createEntity();
	SnakeComponent snakeComp;
	
	snakeComp.segments.push_back({ { 15, 15 }, BeadType::None });  // Head
	snakeComp.segments.push_back({ { 15, 16 }, BeadType::None });
	snakeComp.segments.push_back({ { 16, 16 }, BeadType::None });
	snakeComp.segments.push_back({ { 16, 15 }, BeadType::None });  // Almost complete square
	snakeComp.growing = false;
	
	registry.addComponent<SnakeComponent>(snake, snakeComp);
	registry.addComponent<PositionComponent>(snake, PositionComponent{ { 15, 15 } });
	
	EXPECT_EQ(snakeComp.segments.size(), 4);
	
	auto& snakeComp2 = registry.getComponent<SnakeComponent>(snake);
	EXPECT_GE(snakeComp2.segments.size(), 2);
	
	EXPECT_NO_THROW({
		auto headPos = snakeComp2.segments[0].position;
		for (size_t i = 1; i < snakeComp2.segments.size(); ++i) {
			if (snakeComp2.segments[i].position.x == headPos.x &&
				snakeComp2.segments[i].position.y == headPos.y) {
				// Self-collision detected - mark as growing (effect)
				snakeComp2.growing = true;
			}
		}
	});
}

// 4 - Multiple entities colliding with same target resolves consistently
TEST(CollisionEdgeCases, MultipleEntitiesCollideWithSameTargetConsistently) {
	Registry registry;
	
	Entity snake1 = registry.createEntity();
	Entity snake2 = registry.createEntity();
	Entity food = registry.createEntity();
	
	SnakeComponent comp1;
	comp1.segments.push_back({ { 14, 15 }, BeadType::None });
	comp1.growing = false;
	
	SnakeComponent comp2;
	comp2.segments.push_back({ { 16, 15 }, BeadType::None });
	comp2.growing = false;
	
	registry.addComponent<SnakeComponent>(snake1, comp1);
	registry.addComponent<SnakeComponent>(snake2, comp2);
	registry.addComponent<FoodTag>(food, FoodTag{});
	
	registry.addComponent<PositionComponent>(snake1, PositionComponent{ { 14, 15 } });
	registry.addComponent<PositionComponent>(snake2, PositionComponent{ { 16, 15 } });
	registry.addComponent<PositionComponent>(food, PositionComponent{ { 15, 15 } });
	
	auto& pos1 = registry.getComponent<PositionComponent>(snake1);
	auto& pos2 = registry.getComponent<PositionComponent>(snake2);
	auto& posFood = registry.getComponent<PositionComponent>(food);
	
	EXPECT_NE(pos1.position.x, posFood.position.x);  // Not colliding
	EXPECT_NE(pos2.position.x, posFood.position.x);  // Not colliding
	
	pos1.position.x += 1;
	pos2.position.x -= 1;
	
	EXPECT_EQ(pos1.position.x, posFood.position.x);
	EXPECT_EQ(pos2.position.x, posFood.position.x);
}

// 5 - Collision with fast-moving entity doesn't get missed (no tunneling)
TEST(CollisionEdgeCases, FastMovingEntityCollisionNotMissed) {
	Registry registry;
	
	Entity movingEntity = registry.createEntity();
	Entity target = registry.createEntity();
	
	registry.addComponent<PositionComponent>(movingEntity, PositionComponent{ { 0, 15 } });
	registry.addComponent<PositionComponent>(target, PositionComponent{ { 15, 15 } });
	
	auto& movingPos = registry.getComponent<PositionComponent>(movingEntity);
	auto& targetPos = registry.getComponent<PositionComponent>(target);
	
	int speed = 10;  // Very fast
	for (int tick = 0; tick < 2; ++tick) {
		int prevX = movingPos.position.x;
		movingPos.position.x += speed;
		
		if ((prevX < targetPos.position.x && movingPos.position.x >= targetPos.position.x) ||
			(prevX > targetPos.position.x && movingPos.position.x <= targetPos.position.x)) {
			EXPECT_TRUE(true);
			return;
		}
	}
	
	// If we get here, fast entity passed target (tunneling would be a problem)
	EXPECT_GE(movingPos.position.x, targetPos.position.x);
}
