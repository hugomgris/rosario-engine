#include "AI/Pathfinder.hpp"
#include <set>
#include <algorithm>

void Pathfinder::cleanNodes(std::vector<Node*>& nodes) const {
    for (Node* node : nodes)
        delete node;
    nodes.clear();
}

std::vector<Vec2> Pathfinder::reconstructPath(Node* goalNode) const {
    std::vector<Vec2> path;
    Node* current = goalNode;
    while (current != nullptr && current->parent != nullptr) {
        path.push_back(current->pos);
        current = current->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Vec2> Pathfinder::findPath(const std::vector<std::vector<bool>>& blocked,
                                        Vec2 start,
                                        Vec2 goal,
                                        int gridWidth,
                                        int gridHeight,
                                        int maxDepth,
                                        const std::vector<Vec2>& ignorePositions) const {
    std::multiset<Node*, CompareNode> openList;
    std::vector<std::vector<bool>> visited(gridWidth, std::vector<bool>(gridHeight, false));
    std::vector<Node*> allNodes;

    Node* startNode = new Node{ start, 0, gridHelper.manhattanDistance(start, goal), nullptr };
    openList.insert(startNode);
    allNodes.push_back(startNode);

    int nodesExplored = 0;

    while (!openList.empty() && nodesExplored < maxDepth) {
        ++nodesExplored;
        Node* current = *openList.begin();
        openList.erase(openList.begin());

        if (current->pos.x == goal.x && current->pos.y == goal.y) {
            auto path = reconstructPath(current);
            cleanNodes(allNodes);
            return path;
        }

        if (visited[current->pos.x][current->pos.y]) continue;
        visited[current->pos.x][current->pos.y] = true;

        for (Vec2 neighborPos : gridHelper.getNeighbors(current->pos, gridWidth, gridHeight)) {
            if (!gridHelper.isWalkable(blocked, neighborPos, gridWidth, gridHeight, ignorePositions))
                continue;
            if (visited[neighborPos.x][neighborPos.y])
                continue;

            Node* neighborNode = new Node{
                neighborPos,
                current->gCost + 1,
                gridHelper.manhattanDistance(neighborPos, goal),
                current
            };
            openList.insert(neighborNode);
            allNodes.push_back(neighborNode);
        }
    }

    cleanNodes(allNodes);
    return {};
}
