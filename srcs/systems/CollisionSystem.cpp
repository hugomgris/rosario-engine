#include "systems/CollisionSystem.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "components/CollisionResultComponent.hpp"
#include "arena/ArenaGrid.hpp"

// Resets every snake's CollisionResultComponent to None at the start of each frame
// so that stale results from the previous frame never carry over.
void CollisionSystem::clearResults(Registry& registry) {
    for (auto entity : registry.view<CollisionResultComponent>()) {
        registry.getComponent<CollisionResultComponent>(entity).result = CollisionType::None;
    }
}

// Checks if any snake's head position overlaps with a wall or obstacle.
// When an ArenaGrid is provided we use its O(1) isWalkable lookup directly.
// Without one, we fall back to scanning SolidTag entities (legacy path).
void CollisionSystem::checkWallCollisions(Registry& registry, const ArenaGrid* arena) {
    if (arena) {
        for (auto entity : registry.view<SnakeComponent, PositionComponent, CollisionResultComponent>()) {
            auto& result = registry.getComponent<CollisionResultComponent>(entity);
            if (result.result != CollisionType::None) continue;

            const Vec2 head = registry.getComponent<PositionComponent>(entity).position;
            if (!arena->isWalkable(head.x, head.y))
                result.result = CollisionType::Wall;
        }
        return;
    }

    // Legacy fallback: scan SolidTag entities
    std::vector<Vec2> solidPositions;
    for (auto solid : registry.view<SolidTag, PositionComponent>())
        solidPositions.push_back(registry.getComponent<PositionComponent>(solid).position);

    for (auto entity : registry.view<SnakeComponent, PositionComponent, CollisionResultComponent>()) {
        auto& result = registry.getComponent<CollisionResultComponent>(entity);
        if (result.result != CollisionType::None) continue;

        const Vec2 head = registry.getComponent<PositionComponent>(entity).position;
        for (const auto& solidPos : solidPositions) {
            if (head.x == solidPos.x && head.y == solidPos.y) {
                result.result = CollisionType::Wall;
                break;
            }
        }
    }
}

// Checks if a snake's head overlaps any of its own body segments (index 1 onward).
// Index 0 is the head itself, so we skip it.
void CollisionSystem::checkSelfCollisions(Registry& registry) {
    for (auto entity : registry.view<SnakeComponent, CollisionResultComponent>()) {
        auto& result = registry.getComponent<CollisionResultComponent>(entity);
        if (result.result != CollisionType::None) continue;

        const auto& snake = registry.getComponent<SnakeComponent>(entity);
        if (snake.segments.size() < 2) continue;

        const Vec2 head = snake.segments.front().position;

        for (size_t i = 1; i < snake.segments.size(); ++i) {
            if (head.x == snake.segments[i].position.x &&
                head.y == snake.segments[i].position.y) {
                result.result = CollisionType::Self;
                break;
            }
        }
    }
}

// Checks if a snake's head overlaps any segment of a *different* snake.
void CollisionSystem::checkSnakeCollisions(Registry& registry) {
    auto snakes = registry.view<SnakeComponent, PositionComponent, CollisionResultComponent>();

    for (auto entityA : snakes) {
        auto& result = registry.getComponent<CollisionResultComponent>(entityA);
        if (result.result != CollisionType::None) continue;

        const Vec2 headA = registry.getComponent<PositionComponent>(entityA).position;

        for (auto entityB : snakes) {
            if (entityA == entityB) continue;

            const auto& snakeB = registry.getComponent<SnakeComponent>(entityB);
            for (const auto& seg : snakeB.segments) {
                if (headA.x == seg.position.x && headA.y == seg.position.y) {
                    result.result = CollisionType::Snake;
                    break;
                }
            }
            if (result.result != CollisionType::None) break;
        }
    }
}

// Checks if a snake's head overlaps a food entity.
// On a match, marks the snake as growing and flags the result.
// The food entity is NOT destroyed here — that's the game controller's job,
// which reads CollisionResultComponent and decides when and where to respawn it.
void CollisionSystem::checkFoodCollisions(Registry& registry) {
    std::vector<Vec2> foodPositions;
    for (auto food : registry.view<FoodTag, PositionComponent>()) {
        foodPositions.push_back(registry.getComponent<PositionComponent>(food).position);
    }

    for (auto entity : registry.view<SnakeComponent, PositionComponent, CollisionResultComponent>()) {
        auto& result = registry.getComponent<CollisionResultComponent>(entity);
        if (result.result != CollisionType::None) continue;

        const Vec2 head = registry.getComponent<PositionComponent>(entity).position;

        for (const auto& foodPos : foodPositions) {
            if (head.x == foodPos.x && head.y == foodPos.y) {
                result.result = CollisionType::Food;
                registry.getComponent<SnakeComponent>(entity).growing = true;
                break;
            }
        }
    }
}

void CollisionSystem::update(Registry& registry, const ArenaGrid* arena) {
    clearResults(registry);
    checkWallCollisions(registry, arena);
    checkSelfCollisions(registry);
    checkSnakeCollisions(registry);
    checkFoodCollisions(registry);
}
