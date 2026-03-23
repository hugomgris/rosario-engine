#include "systems/AISystem.hpp"
#include "components/AIComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include <cstdlib>

BlockedGrid AISystem::buildBlockedGrid(Registry& registry, const ArenaGrid* arena) const {
    BlockedGrid blocked(_gridWidth, std::vector<bool>(_gridHeight, false));

    if (arena) {
        // Mark everything the arena considers non-walkable
        for (int x = 0; x < _gridWidth; ++x)
            for (int y = 0; y < _gridHeight; ++y)
                if (!arena->isWalkable(x, y))
                    blocked[x][y] = true;
    } else {
        for (auto e : registry.view<SolidTag, PositionComponent>()) {
            const auto& p = registry.getComponent<PositionComponent>(e).position;
            if (p.x >= 0 && p.x < _gridWidth && p.y >= 0 && p.y < _gridHeight)
                blocked[p.x][p.y] = true;
        }
    }

    for (auto e : registry.view<SnakeComponent>()) {
        for (const auto& seg : registry.getComponent<SnakeComponent>(e).segments)
            if (seg.position.x >= 0 && seg.position.x < _gridWidth &&
                seg.position.y >= 0 && seg.position.y < _gridHeight)
                blocked[seg.position.x][seg.position.y] = true;
    }
    return blocked;
}

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
    return _gridHelper.isWalkable(blocked, nextPos, _gridWidth, _gridHeight);
}

Direction AISystem::goToFood(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const auto& pos   = registry.getComponent<PositionComponent>(entity).position;
    const auto& ai    = registry.getComponent<AIComponent>(entity);

    Vec2 foodPos = { -1, -1 };
    for (auto food : registry.view<FoodTag, PositionComponent>()) {
        foodPos = registry.getComponent<PositionComponent>(food).position;
        break;
    }
    if (foodPos.x == -1) return Direction::UP;

    auto path = _pathFinder.findPath(blocked, pos, foodPos, _gridWidth, _gridHeight, ai.maxSearchDepth);
    if (path.empty())
        return survivalMode(registry, entity, blocked);

    if (ai.useSafetyCheck && !_floodFill.canReachTail(registry, blocked, entity, path, _gridWidth, _gridHeight)) {
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
    auto path = _pathFinder.findPath(blocked, pos, tail, _gridWidth, _gridHeight, ai.maxSearchDepth);
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

        int space = _floodFill.countReachable(blocked, next, _gridWidth, _gridHeight);
        if (space > bestSpace) {
            bestSpace = space;
            bestDir   = dir;
        }
    }
    return bestDir;
}

Direction AISystem::decideDirection(Registry& registry, Entity entity, const BlockedGrid& blocked) {
    const auto& ai = registry.getComponent<AIComponent>(entity);

    if (ai.behavior == AIBehaviourState::EASY) {
        float roll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        if (roll < ai.randomMoveChance)
            return maximizeSpace(registry, entity, blocked);
    }

    return goToFood(registry, entity, blocked);
}

void AISystem::update(Registry& registry, const FrameContext& ctx) {
    for (auto entity : registry.view<AIComponent, MovementComponent, PositionComponent, SnakeComponent>()) {
        const BlockedGrid blocked = buildBlockedGrid(registry, ctx.arena);
        Direction dir = decideDirection(registry, entity, blocked);
        registry.getComponent<MovementComponent>(entity).direction = dir;
    }
}
