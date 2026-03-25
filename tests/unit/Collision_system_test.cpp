#include <gtest/gtest.h>

#include <string>

#include "collision/CollisionRule.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "systems/CollisionSystem.hpp"
#include "components/SnakeComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/ScoreComponent.hpp"
#include "components/FoodTag.hpp"

namespace {
void MarkSnakeGrowing(Registry& registry, Entity subject, Entity, FrameContext&) {
	if (!registry.hasComponent<SnakeComponent>(subject)) {
		return;
	}
	registry.getComponent<SnakeComponent>(subject).growing = true;
}

void AddScore(Registry& registry, Entity subject, Entity, FrameContext&) {
	if (!registry.hasComponent<ScoreComponent>(subject)) {
		return;
	}
	registry.getComponent<ScoreComponent>(subject).score += 1;
}
}

TEST(CollisionRules, StructConstructionAndFieldAssignment) {
	CollisionRule rule;
	rule.subject = "Snake";
	rule.object = "Food";
	rule.effects = { "GrowSnake", "IncrementScore" };

	EXPECT_EQ(rule.subject, "Snake");
	EXPECT_EQ(rule.object, "Food");
	ASSERT_EQ(rule.effects.size(), 2);
	EXPECT_EQ(rule.effects[0], "GrowSnake");
	EXPECT_EQ(rule.effects[1], "IncrementScore");
}

TEST(CollisionRules, FindReturnsMatchingRuleForSubjectObjectPair) {
	CollisionRuleTable table;
	table.rules.push_back({ "Snake", "Food", { "GrowSnake" } });

	const CollisionRule* rule = table.find("Snake", "Food");
	ASSERT_NE(rule, nullptr);
	EXPECT_EQ(rule->effects.size(), 1);
	EXPECT_EQ(rule->effects[0], "GrowSnake");
}

TEST(CollisionRules, FindReturnsNullptrForUnknownPair) {
	CollisionRuleTable table;
	table.rules.push_back({ "Snake", "Food", { "GrowSnake" } });

	EXPECT_EQ(table.find("Snake", "Wall"), nullptr);
	EXPECT_EQ(table.find("Food", "Snake"), nullptr);
}

TEST(CollisionEffectDispatcher, ExecuteAppliesRegisteredEffect) {
	Registry registry;
	Entity snake = registry.createEntity();
	registry.addComponent<SnakeComponent>(snake, SnakeComponent{});

	CollisionEffectDispatcher dispatcher;
	dispatcher.registerEffect("GrowSnake", MarkSnakeGrowing);

	FrameContext ctx;
	dispatcher.execute("GrowSnake", registry, snake, snake, ctx);

	EXPECT_TRUE(registry.getComponent<SnakeComponent>(snake).growing);
}

TEST(CollisionEffectDispatcher, MultipleEffectsCanApplyOnSingleCollision) {
	Registry registry;
	CollisionSystem collisionSystem;
	CollisionEffectDispatcher dispatcher;
	dispatcher.registerEffect("GrowSnake", MarkSnakeGrowing);
	dispatcher.registerEffect("IncrementScore", AddScore);

	CollisionRuleTable table;
	table.rules.push_back({ "Snake", "Food", { "GrowSnake", "IncrementScore" } });

	Entity snake = registry.createEntity();
	SnakeComponent snakeData;
	snakeData.segments.push_back({ { 5, 5 }, BeadType::None });
	registry.addComponent<SnakeComponent>(snake, snakeData);
	registry.addComponent<PositionComponent>(snake, PositionComponent{ { 5, 5 } });
	registry.addComponent<ScoreComponent>(snake, ScoreComponent{});

	Entity food = registry.createEntity();
	registry.addComponent<FoodTag>(food, FoodTag{});
	registry.addComponent<PositionComponent>(food, PositionComponent{ { 5, 5 } });

	FrameContext ctx;
	collisionSystem.update(registry, table, dispatcher, ctx);

	EXPECT_TRUE(registry.getComponent<SnakeComponent>(snake).growing);
	EXPECT_EQ(registry.getComponent<ScoreComponent>(snake).score, 1);
}

TEST(CollisionEffectDispatcher, ExecuteThrowsForUnknownEffectName) {
	Registry registry;
	Entity e = registry.createEntity();

	CollisionEffectDispatcher dispatcher;
	FrameContext ctx;

	EXPECT_THROW(dispatcher.execute("UnknownEffect", registry, e, e, ctx), std::runtime_error);
}
