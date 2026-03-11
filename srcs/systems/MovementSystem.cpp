#include "systems/MovementSystem.hpp"
#include "../components/MovementComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SnakeComponent.hpp"
#include <iostream>

Vec2 MovementSystem::directionToVec2(Direction dir) const {
	switch (dir) {
		case Direction::UP:		return {  0, -1 };
		case Direction::DOWN:	return {  0,  1 };
		case Direction::LEFT:	return { -1,  0 };
		case Direction::RIGHT:	return {  1,  0 };
	}
	return { 0, 0 };
}

// reads each entity's InputComponent and updates its Movement Component direction.
// ignores inputs that would reverse the current direction and discards invalid buffers (silently)
// lastDirection is the direction the snake last actually moved (not the buffered one)
void MovementSystem::processInput(Registry& registry, Direction lastDirection, MovementComponent& move, InputComponent& input) {
	while (!input.inputBuffer.empty()) {
		const Input next = input.inputBuffer.front();
		input.inputBuffer.pop();

		Direction requested = move.direction;

		switch (next) {
			case Input::Up_A:    requested = Direction::UP;    break;
			case Input::Down_A:  requested = Direction::DOWN;  break;
			case Input::Left_A:  requested = Direction::LEFT;  break;
			case Input::Right_A: requested = Direction::RIGHT; break;
			case Input::Up_B:    requested = Direction::UP;    break;
			case Input::Down_B:  requested = Direction::DOWN;  break;
			case Input::Left_B:  requested = Direction::LEFT;  break;
			case Input::Right_B: requested = Direction::RIGHT; break;
			default: break;
		}

		const Vec2 lastVec      = directionToVec2(lastDirection);
		const Vec2 requestedVec = directionToVec2(requested);

		bool isReversal = (lastVec.x + requestedVec.x == 0) &&
						  (lastVec.y + requestedVec.y == 0);

		if (!isReversal) {
			move.direction = requested;
			break; // only apply one valid direction per move tick
		}
	}
}

// advances the move timer for each snake entity.
// when timer exceeds interval, consumes input and moves one grid cell
void MovementSystem::advanceSnake(Registry& registry, float deltaTime) {
	for (auto entity : registry.view<MovementComponent, PositionComponent, SnakeComponent>()) {
		auto& move	= registry.getComponent<MovementComponent>(entity);
		auto& pos	= registry.getComponent<PositionComponent>(entity);
		auto& snake	= registry.getComponent<SnakeComponent>(entity);

		move.moveTimer += deltaTime;
		if (move.moveTimer < move.moveInterval)
			continue;
		move.moveTimer = 0.0f;

		if (snake.segments.empty())
			continue;

		// consume input only when the snake is about to move,
		// using the last committed direction as the reversal reference
		if (registry.hasComponent<InputComponent>(entity)) {
			auto& input = registry.getComponent<InputComponent>(entity);
			processInput(registry, move.direction, move, input);
		}

		const Vec2 delta	= directionToVec2(move.direction);
		const Vec2 newHead	= { snake.segments.front().position.x + delta.x,
								snake.segments.front().position.y + delta.y };

		snake.segments.push_front({ newHead, BeadType::None });
		pos.position = newHead;

		if (snake.growing) {
			snake.growing = false;
		} else {
			snake.segments.pop_back();
		}
	}
}

void MovementSystem::update(Registry& registry, float deltaTime) {
	advanceSnake(registry, deltaTime);
}