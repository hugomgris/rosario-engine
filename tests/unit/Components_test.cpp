#include <gtest/gtest.h>

#include "components/SnakeComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/ScoreComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/RenderComponent.hpp"

TEST(Components, SnakeComponentCanAddSegments) {
	SnakeComponent snake;
	snake.segments.push_back({ { 1, 1 }, BeadType::None });
	snake.segments.push_back({ { 2, 1 }, BeadType::None });

	EXPECT_EQ(snake.segments.size(), 2);
	EXPECT_EQ(snake.segments.front().position.x, 1);
	EXPECT_EQ(snake.segments.back().position.x, 2);
}

TEST(Components, SnakeComponentCanRemoveSegments) {
	SnakeComponent snake;
	snake.segments.push_back({ { 1, 1 }, BeadType::None });
	snake.segments.push_back({ { 2, 1 }, BeadType::None });
	snake.segments.push_back({ { 3, 1 }, BeadType::None });

	snake.segments.pop_back();
	EXPECT_EQ(snake.segments.size(), 2);
	EXPECT_EQ(snake.segments.back().position.x, 2);
}

TEST(Components, SnakeComponentSegmentCountMatchesMutations) {
	SnakeComponent snake;
	EXPECT_EQ(snake.segments.size(), 0);

	snake.segments.push_back({ { 4, 4 }, BeadType::None });
	snake.segments.push_back({ { 5, 4 }, BeadType::None });
	EXPECT_EQ(snake.segments.size(), 2);

	snake.segments.pop_front();
	EXPECT_EQ(snake.segments.size(), 1);
}

TEST(Components, SnakeComponentSupportsPowerupBeadAssignment) {
	Segment segment;
	segment.position = { 2, 3 };

	segment.bead = BeadType::Shield;
	EXPECT_EQ(segment.bead, BeadType::Shield);

	segment.bead = BeadType::Speed;
	EXPECT_EQ(segment.bead, BeadType::Speed);

	segment.bead = BeadType::Ghost;
	EXPECT_EQ(segment.bead, BeadType::Ghost);

	segment.bead = BeadType::Ram;
	EXPECT_EQ(segment.bead, BeadType::Ram);
}

TEST(Components, BeadTypeEnumValuesAreDistinct) {
	EXPECT_NE(static_cast<int>(BeadType::None), static_cast<int>(BeadType::Shield));
	EXPECT_NE(static_cast<int>(BeadType::Shield), static_cast<int>(BeadType::Speed));
	EXPECT_NE(static_cast<int>(BeadType::Speed), static_cast<int>(BeadType::Ghost));
	EXPECT_NE(static_cast<int>(BeadType::Ghost), static_cast<int>(BeadType::Ram));
}

TEST(Components, PositionComponentStoresGridCoordinates) {
	PositionComponent position{ { 7, 9 } };
	EXPECT_EQ(position.position.x, 7);
	EXPECT_EQ(position.position.y, 9);
}

TEST(Components, ScoreComponentCanBeIncremented) {
	ScoreComponent score;
	EXPECT_EQ(score.score, 0);

	score.score += 1;
	score.score += 2;
	EXPECT_EQ(score.score, 3);
}

TEST(Components, MovementComponentDirectionAssignmentWorks) {
	MovementComponent movement{ Direction::UP, 0.0f, 0.1f };
	EXPECT_EQ(movement.direction, Direction::UP);

	movement.direction = Direction::LEFT;
	EXPECT_EQ(movement.direction, Direction::LEFT);

	movement.direction = Direction::DOWN;
	EXPECT_EQ(movement.direction, Direction::DOWN);

	movement.direction = Direction::RIGHT;
	EXPECT_EQ(movement.direction, Direction::RIGHT);
}

TEST(Components, InputComponentQueueOperationsWork) {
	InputComponent input;
	EXPECT_TRUE(input.inputBuffer.empty());

	input.inputBuffer.push(Input::Up_A);
	input.inputBuffer.push(Input::Left_A);
	ASSERT_EQ(input.inputBuffer.size(), 2);

	EXPECT_EQ(input.inputBuffer.front(), Input::Up_A);
	input.inputBuffer.pop();
	EXPECT_EQ(input.inputBuffer.front(), Input::Left_A);
}

TEST(Components, RenderComponentStoresTintColor) {
	BaseColor tint{ 10, 20, 30, 255 };
	RenderComponent render{ tint };

	EXPECT_EQ(render.color.r, 10);
	EXPECT_EQ(render.color.g, 20);
	EXPECT_EQ(render.color.b, 30);
	EXPECT_EQ(render.color.a, 255);
}
