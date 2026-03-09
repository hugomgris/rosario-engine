#include "systems/MovementSystem.hpp"
#include "components/MovementComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"

Vec2 MovementSystem::directionToVec2(Direction dir) const {
    switch (dir) {
        case Direction::UP:    return {  0, -1 };
        case Direction::DOWN:  return {  0,  1 };
        case Direction::LEFT:  return { -1,  0 };
        case Direction::RIGHT: return {  1,  0 };
    }
    return { 0, 0 };
}

// Reads each entity's InputComponent and updates its MovementComponent direction.
// Ignores inputs that would reverse the current direction (a snake can't go back
// on itself) and discards invalid/empty buffers silently.
void MovementSystem::processInput(Registry& registry) {
    for (auto entity : registry.view<InputComponent, MovementComponent>()) {
        auto& input = registry.getComponent<InputComponent>(entity);
        auto& mov   = registry.getComponent<MovementComponent>(entity);

        if (input.inputBuffer.empty())
            continue;

        const Input next = input.inputBuffer.front();
        input.inputBuffer.pop();

        Direction requested = mov.direction; // default: keep current

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

        // Prevent 180-degree reversal
        const Vec2 currentVec  = directionToVec2(mov.direction);
        const Vec2 requestedVec = directionToVec2(requested);
        const bool isReversal  = (currentVec.x + requestedVec.x == 0) &&
                                  (currentVec.y + requestedVec.y == 0);

        if (!isReversal)
            mov.direction = requested;
    }
}

// Advances the move timer for each snake entity. When the timer exceeds the
// move interval, it moves the snake one grid cell in its current direction:
// the head shifts forward, each body segment shifts into the previous segment's
// position, and the tail is dropped unless the snake is currently growing.
void MovementSystem::advanceSnake(Registry& registry, float deltaTime) {
    for (auto entity : registry.view<MovementComponent, PositionComponent, SnakeComponent>()) {
        auto& mov   = registry.getComponent<MovementComponent>(entity);
        auto& pos   = registry.getComponent<PositionComponent>(entity);
        auto& snake = registry.getComponent<SnakeComponent>(entity);

        mov.moveTimer += deltaTime;
        if (mov.moveTimer < mov.moveInterval)
            continue;
        mov.moveTimer = 0.0f; // reset cleanly — never accumulate multiple ticks

        if (snake.segments.empty())
            continue;

        const Vec2 delta   = directionToVec2(mov.direction);
        const Vec2 newHead = { snake.segments.front().position.x + delta.x,
                               snake.segments.front().position.y + delta.y };

        // Push new head at front, drop tail at back unless growing.
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
    processInput(registry);
    advanceSnake(registry, deltaTime);
}
