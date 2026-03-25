#include "AI/FloodFill.hpp"
#include "components/SnakeComponent.hpp"
#include <cstdlib>
#include <queue>

namespace {
bool samePosition(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

bool isAdjacentCardinal(Vec2 a, Vec2 b) {
    const int dx = std::abs(a.x - b.x);
    const int dy = std::abs(a.y - b.y);
    return (dx + dy) == 1;
}
}

int FloodFill::countReachable(const std::vector<std::vector<bool>>& blocked,
                               Vec2 start,
                               int gridWidth,
                               int gridHeight,
                               const std::vector<Vec2>& ignorePositions) const {
    if (!gridHelper.isWalkable(blocked, start, gridWidth, gridHeight, ignorePositions))
        return 0;

    std::vector<std::vector<bool>> visited(gridWidth, std::vector<bool>(gridHeight, false));
    std::queue<Vec2> queue;
    queue.push(start);
    visited[start.x][start.y] = true;
    int count = 0;

    while (!queue.empty()) {
        Vec2 current = queue.front();
        queue.pop();
        ++count;

        for (Vec2 neighbor : gridHelper.getNeighbors(current, gridWidth, gridHeight)) {
            if (!gridHelper.isWalkable(blocked, neighbor, gridWidth, gridHeight, ignorePositions))
                continue;
            if (visited[neighbor.x][neighbor.y])
                continue;
            visited[neighbor.x][neighbor.y] = true;
            queue.push(neighbor);
        }
    }
    return count;
}

bool FloodFill::canReachTail(const Registry& registry,
                              const std::vector<std::vector<bool>>& blocked,
                              Entity aiEntity,
                              const std::vector<Vec2>& proposedPath,
                              int gridWidth,
                              int gridHeight) const {
    if (proposedPath.empty())
        return false;

    const auto& snake = registry.getComponent<SnakeComponent>(aiEntity);
    if (snake.segments.empty())
        return false;

    std::deque<Segment> futureSegments = snake.segments;
    bool growsThisStep = snake.growing;

    for (const Vec2& nextHead : proposedPath) {
        if (!isAdjacentCardinal(futureSegments.front().position, nextHead))
            return false;

        futureSegments.push_front({ nextHead, BeadType::None });
        if (growsThisStep) {
            growsThisStep = false;
        } else {
            futureSegments.pop_back();
        }
    }

    const Vec2 futureHead = futureSegments.front().position;
    const Vec2 futureTail = futureSegments.back().position;

    std::vector<std::vector<bool>> futureBlocked = blocked;
    for (const auto& seg : snake.segments) {
        const Vec2& pos = seg.position;
        if (pos.x >= 0 && pos.x < gridWidth && pos.y >= 0 && pos.y < gridHeight)
            futureBlocked[pos.x][pos.y] = false;
    }
    for (std::size_t i = 0; i < futureSegments.size(); ++i) {
        if (i == 0)
            continue;

        const Vec2& pos = futureSegments[i].position;
        if (pos.x >= 0 && pos.x < gridWidth && pos.y >= 0 && pos.y < gridHeight)
            futureBlocked[pos.x][pos.y] = true;
    }

    if (samePosition(futureHead, futureTail))
        return true;

    std::vector<std::vector<bool>> visited(gridWidth, std::vector<bool>(gridHeight, false));
    std::queue<Vec2> queue;
    queue.push(futureHead);
    visited[futureHead.x][futureHead.y] = true;

    while (!queue.empty()) {
        const Vec2 current = queue.front();
        queue.pop();

        for (Vec2 neighbor : gridHelper.getNeighbors(current, gridWidth, gridHeight)) {
            if (!gridHelper.isWalkable(futureBlocked, neighbor, gridWidth, gridHeight, { futureTail }))
                continue;
            if (visited[neighbor.x][neighbor.y])
                continue;
            if (samePosition(neighbor, futureTail)) {
                const int reachable = countReachable(futureBlocked,
                                                     futureHead,
                                                     gridWidth,
                                                     gridHeight,
                                                     { futureTail });
                const int required = static_cast<int>(futureSegments.size()) + 1;
                return reachable >= required;
            }

            visited[neighbor.x][neighbor.y] = true;
            queue.push(neighbor);
        }
    }

    return false;
}
