#include "systems/AISystem.hpp"
#include "components/AIComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include <cstdlib>

// ─── Grid construction ───────────────────────────────────────────────────────

// Builds the blocked[x][y] boolean grid once per AI tick.
// When an ArenaGrid is provided, we use its O(1) isWalkable query.
// Without one, we fall back to scanning SolidTag entities.
// In both cases, all snake body segments are also marked blocked.
BlockedGrid AISystem::buildBlockedGrid(Registry& registry, const ArenaGrid* arena) const {
    BlockedGrid blocked(gridWidth, std::vector<bool>(gridHeight, false));

    if (arena) {
        // Mark everything the arena considers non-walkable
        for (int x = 0; x < gridWidth; ++x)
            for (int y = 0; y < gridHeight; ++y)
                if (!arena->isWalkable(x, y))
                    blocked[x][y] = true;
    } else {
        for (auto e : registry.view<SolidTag, PositionComponent>()) {
            const auto& p = registry.getComponent<PositionComponent>(e).position;
            if (p.x >= 0 && p.x < gridWidth && p.y >= 0 && p.y < gridHeight)
                blocked[p.x][p.y] = true;
        }
    }

    for (auto e : registry.view<SnakeComponent>()) {
        for (const auto& seg : registry.getComponent<SnakeComponent>(e).segments)
            if (seg.position.x >= 0 && seg.position.x < gridWidth &&
                seg.position.y >= 0 && seg.position.y < gridHeight)
                blocked[seg.position.x][seg.position.y] = true;
    }
    return blocked;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

Direction AISystem::vecToDirection(Vec2 from, Vec2 to) const {
    const int dx = to.x - from.x;
    const int dy = to.y - from.y;
    if (dx > 0) return Direction::RIGHT;
    if (dx < 0) return Direction::LEFT;
    if (dy > 0) return Direction::DOWN;
    return Direction::UP;
}

Vec2 AISystem::stepInDirection(Vec2 pos, Direction dir) const {
    switch (dir) {
        case Direction::UP:    return { pos.x,     pos.y - 1 };
        case Direction::DOWN:  return { pos.x,     pos.y + 1 };
        case Direction::LEFT:  return { pos.x - 1, pos.y     };
        case Direction::RIGHT: return { pos.x + 1, pos.y     };
    }
    return pos;
}

bool AISystem::isSafeMove(const BlockedGrid& blocked, Vec2 nextPos) const {
    return gridHelper.isWalkable(blocked, nextPos, gridWidth, gridHeight);
}

// ─── Decision steps ──────────────────────────────────────────────────────────

Direction AISystem::goToFood(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const auto& pos   = registry.getComponent<PositionComponent>(entity).position;
    const auto& ai    = registry.getComponent<AIComponent>(entity);

    Vec2 foodPos = { -1, -1 };
    for (auto food : registry.view<FoodTag, PositionComponent>()) {
        foodPos = registry.getComponent<PositionComponent>(food).position;
        break;
    }
    if (foodPos.x == -1) return Direction::UP;

    auto path = pathfinder.findPath(blocked, pos, foodPos, gridWidth, gridHeight, ai.maxSearchDepth);
    if (path.empty())
        return survivalMode(registry, entity, blocked);

    if (ai.useSafetyCheck && !floodFill.canReachTail(registry, blocked, entity, path, gridWidth, gridHeight)) {
        if (ai.hasSurvivalMode)
            return survivalMode(registry, entity, blocked);
    }

    return vecToDirection(pos, path.front());
}

Direction AISystem::survivalMode(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const auto& pos   = registry.getComponent<PositionComponent>(entity).position;
    const auto& ai    = registry.getComponent<AIComponent>(entity);
    const auto& snake = registry.getComponent<SnakeComponent>(entity);

    if (snake.segments.empty())
        return maximizeSpace(registry, entity, blocked);

    Vec2 tail = snake.segments.back().position;
    auto path = pathfinder.findPath(blocked, pos, tail, gridWidth, gridHeight, ai.maxSearchDepth);
    if (!path.empty())
        return vecToDirection(pos, path.front());

    return maximizeSpace(registry, entity, blocked);
}

Direction AISystem::maximizeSpace(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const Vec2 pos = registry.getComponent<PositionComponent>(entity).position;

    Direction bestDir   = Direction::UP;
    int       bestSpace = -1;

    for (Direction dir : { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT }) {
        Vec2 next = stepInDirection(pos, dir);
        if (!isSafeMove(blocked, next)) continue;

        int space = floodFill.countReachable(blocked, next, gridWidth, gridHeight);
        if (space > bestSpace) {
            bestSpace = space;
            bestDir   = dir;
        }
    }
    return bestDir;
}

// ─── Main decision ────────────────────────────────────────────────────────────

Direction AISystem::decideDirection(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const auto& ai = registry.getComponent<AIComponent>(entity);

    if (ai.behavior == AIBehaviourState::EASY) {
        float roll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        if (roll < ai.randomMoveChance)
            return maximizeSpace(registry, entity, blocked);
    }

    return goToFood(registry, entity, blocked);
}

// ─── System update ────────────────────────────────────────────────────────────

void AISystem::update(Registry& registry, const ArenaGrid* arena) {
    for (auto entity : registry.view<AIComponent, MovementComponent, PositionComponent, SnakeComponent>()) {
        // Build blocked grid once per AI entity per frame.
        // All A* and flood-fill calls within decideDirection reuse this grid.
        const BlockedGrid blocked = buildBlockedGrid(registry, arena);
        Direction dir = decideDirection(registry, entity, blocked);
        registry.getComponent<MovementComponent>(entity).direction = dir;
    }
}
