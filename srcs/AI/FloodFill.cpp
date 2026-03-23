#include "AI/FloodFill.hpp"
#include "components/SnakeComponent.hpp"
#include <queue>

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

    const Vec2 newHead     = proposedPath.back();
    const Vec2 currentTail = snake.segments.back().position;

    int reachable     = countReachable(blocked, newHead, gridWidth, gridHeight, { currentTail });
    const int required = static_cast<int>(snake.segments.size()) + 1;
    return reachable >= required;
}
