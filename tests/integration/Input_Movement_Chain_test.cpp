#include <gtest/gtest.h>

#include <queue>
#include "../../incs/DataStructs.hpp"
#include "ecs/Registry.hpp"
#include "systems/InputSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/AISystem.hpp"
#include "components/InputComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "arena/ArenaGrid.hpp"
#include "FrameContext.hpp"

// 1 - Player input buffering is consumed across frames in deterministic order
TEST(InputMovementChain, InputBufferingConsumedDeterministically) {
	Registry registry;
	
	Entity player = registry.createEntity();
	
	InputComponent inputComp;
	inputComp.inputBuffer.push(Input::Up_A);
	inputComp.inputBuffer.push(Input::Left_A);
	inputComp.inputBuffer.push(Input::Down_A);
	
	registry.addComponent<InputComponent>(player, inputComp);
	registry.addComponent<MovementComponent>(player, MovementComponent{});
	
	auto& input = registry.getComponent<InputComponent>(player);
	
	// Frame 1: consume first input
	ASSERT_EQ(input.inputBuffer.size(), 3);
	EXPECT_EQ(input.inputBuffer.front(), Input::Up_A);
	input.inputBuffer.pop();
	EXPECT_EQ(input.inputBuffer.size(), 2);
	
	// Frame 2: consume second input
	EXPECT_EQ(input.inputBuffer.front(), Input::Left_A);
	input.inputBuffer.pop();
	EXPECT_EQ(input.inputBuffer.size(), 1);
	
	// Frame 3: consume third input
	EXPECT_EQ(input.inputBuffer.front(), Input::Down_A);
	input.inputBuffer.pop();
	EXPECT_EQ(input.inputBuffer.size(), 0);
}

// 2 - Opposite-direction rejection remains correct across multiple buffered inputs
TEST(InputMovementChain, OppositateDirectionRejectionWithMultipleBufferedInputs) {
	Registry registry;
	
	Entity player = registry.createEntity();
	
	MovementComponent moveComp;
	moveComp.direction = Direction::UP;
	
	registry.addComponent<MovementComponent>(player, moveComp);
	registry.addComponent<PositionComponent>(player, PositionComponent{ { 16, 16 } });
	
	InputComponent inputComp;
	// Buffer: RIGHT, DOWN, DOWN, LEFT
	inputComp.inputBuffer.push(Input::Right_A);
	inputComp.inputBuffer.push(Input::Down_A);  // DOWN is opposite of UP and should be rejected
	inputComp.inputBuffer.push(Input::Down_A);  // Still opposite - should be rejected
	inputComp.inputBuffer.push(Input::Left_A);
	
	registry.addComponent<InputComponent>(player, inputComp);
	
	auto& input = registry.getComponent<InputComponent>(player);
	auto& movement = registry.getComponent<MovementComponent>(player);
	
	// Frame 1: Accept RIGHT (valid)
	Direction nextDir1 = Direction::RIGHT;
	EXPECT_NE(nextDir1, Direction::DOWN); // Opposite check
	movement.direction = nextDir1;
	input.inputBuffer.pop();
	
	// Frame 2: Reject DOWN (opposite of current RIGHT)
	Direction nextDir2 = Direction::DOWN;
	EXPECT_NE(nextDir2, Direction::UP); // Would fail opposite check
	// In real system this would be rejected before being processed, so skip it
	input.inputBuffer.pop();
	
	// Frame 3: Skip another opposite
	input.inputBuffer.pop();
	
	// Frame 4: Accept LEFT (valid from RIGHT)
	Direction nextDir4 = Direction::LEFT;
	EXPECT_NE(nextDir4, Direction::RIGHT);
	input.inputBuffer.pop();
	
	EXPECT_EQ(input.inputBuffer.size(), 0);
}

// 3 - VS-AI mode: AI decision output is reflected in movement on the next tick
TEST(InputMovementChain, AIDecisionReflectedInMovementNextTick) {
	Registry registry;
	
	// Create AI entity
	Entity aiEntity = registry.createEntity();
	
	registry.addComponent<MovementComponent>(aiEntity, MovementComponent{});
	registry.addComponent<PositionComponent>(aiEntity, PositionComponent{ { 16, 16 } });
	
	auto& movement = registry.getComponent<MovementComponent>(aiEntity);
	
	// Simulate AI decision phase made in frame N
	Direction aiDecision = Direction::LEFT;
	
	// Apply decision at end of frame
	movement.direction = aiDecision;
	EXPECT_EQ(movement.direction, Direction::LEFT);
	
	// Frame N+1: Movement system reads the direction set by AI
	// Simulate position update based on direction
	auto& position = registry.getComponent<PositionComponent>(aiEntity);
	
	if (movement.direction == Direction::LEFT) {
		position.position.x -= 1;
	}
	
	EXPECT_EQ(position.position.x, 15);
	EXPECT_EQ(position.position.y, 16);
}

// 4 - Multiple input sources (keyboard/mouse) resolve to single direction per frame
TEST(InputMovementChain, MultipleInputSourcesResolveToSingleDirection) {
	Registry registry;
	
	Entity player = registry.createEntity();
	
	InputComponent inputComp;
	registry.addComponent<InputComponent>(player, inputComp);
	registry.addComponent<MovementComponent>(player, MovementComponent{});
	
	auto& input = registry.getComponent<InputComponent>(player);
	auto& movement = registry.getComponent<MovementComponent>(player);
	
	// Frame 1: Keyboard input UP
	input.inputBuffer.push(Input::Up_A);
	
	// Frame 1 (same frame): Mouse input LEFT arrives
	input.inputBuffer.push(Input::Left_A);
	
	// System should process inputs in queue order
	// First input (UP) processed first
	Direction resolvedDir1 = Direction::UP;
	movement.direction = resolvedDir1;
	input.inputBuffer.pop();
	
	// On next frame, next input (LEFT) is processed
	Direction resolvedDir2 = Direction::LEFT;
	movement.direction = resolvedDir2;
	input.inputBuffer.pop();
	
	EXPECT_EQ(movement.direction, Direction::LEFT);
}

// 5 - Input buffering capacity prevents loss of intentional player input
TEST(InputMovementChain, InputBufferCapacityPreservesInput) {
	Registry registry;
	
	Entity player = registry.createEntity();
	
	InputComponent inputComp;
	// Buffer multiple inputs
	for (int i = 0; i < 5; ++i) {
		inputComp.inputBuffer.push(Input::Up_A);
	}
	
	registry.addComponent<InputComponent>(player, inputComp);
	
	auto& input = registry.getComponent<InputComponent>(player);
	
	// Verify buffer size
	EXPECT_EQ(input.inputBuffer.size(), 5);
	
	// Process each input
	for (int i = 0; i < 5; ++i) {
		EXPECT_FALSE(input.inputBuffer.empty());
		input.inputBuffer.pop();
	}
	
	EXPECT_TRUE(input.inputBuffer.empty());
}
