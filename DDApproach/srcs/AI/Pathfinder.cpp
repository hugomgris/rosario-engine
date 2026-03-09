#include "Pathfinder.hpp"
#include "GridHelper.hpp"
#include "FloodFill.hpp"
#include <vector>
#include <algorithm>

Pathfinder::Pathfinder(const Grid& grid) : grid(grid) {}

std::vector<Vector2> Pathfinder::findPath(const Vector2& start, const Vector2& goal) {
    std::vector<Vector2> path;
    if (!isValidPosition(start) || !isValidPosition(goal)) {
        return path; // Return empty path if start or goal is invalid
    }

    std::vector<Vector2> openSet;
    std::set<Vector2> closedSet;
    openSet.push_back(start);

    std::map<Vector2, Vector2> cameFrom;
    std::map<Vector2, float> gScore;
    std::map<Vector2, float> fScore;

    gScore[start] = 0;
    fScore[start] = heuristic(start, goal);

    while (!openSet.empty()) {
        Vector2 current = getLowestFScore(openSet, fScore);
        if (current == goal) {
            path = reconstructPath(cameFrom, current);
            break;
        }

        openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
        closedSet.insert(current);

        for (const Vector2& neighbor : getNeighbors(current)) {
            if (closedSet.find(neighbor) != closedSet.end()) {
                continue; // Ignore the neighbor which is already evaluated
            }

            float tentativeGScore = gScore[current] + distance(current, neighbor);
            if (std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end()) {
                openSet.push_back(neighbor); // Discover a new node
            } else if (tentativeGScore >= gScore[neighbor]) {
                continue; // This is not a better path
            }

            cameFrom[neighbor] = current;
            gScore[neighbor] = tentativeGScore;
            fScore[neighbor] = gScore[neighbor] + heuristic(neighbor, goal);
        }
    }

    return path;
}

bool Pathfinder::isValidPosition(const Vector2& position) const {
    return grid.isWalkable(position.x, position.y);
}

Vector2 Pathfinder::getLowestFScore(const std::vector<Vector2>& openSet, const std::map<Vector2, float>& fScore) const {
    return *std::min_element(openSet.begin(), openSet.end(), [&fScore](const Vector2& a, const Vector2& b) {
        return fScore.at(a) < fScore.at(b);
    });
}

std::vector<Vector2> Pathfinder::getNeighbors(const Vector2& position) const {
    std::vector<Vector2> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue; // Skip the current position
            if (std::abs(dx) + std::abs(dy) == 1) { // Only consider orthogonal neighbors
                Vector2 neighbor(position.x + dx, position.y + dy);
                if (isValidPosition(neighbor)) {
                    neighbors.push_back(neighbor);
                }
            }
        }
    }
    return neighbors;
}

std::vector<Vector2> Pathfinder::reconstructPath(const std::map<Vector2, Vector2>& cameFrom, const Vector2& current) const {
    std::vector<Vector2> totalPath = { current };
    auto it = cameFrom.find(current);
    while (it != cameFrom.end()) {
        totalPath.push_back(it->second);
        it = cameFrom.find(it->second);
    }
    std::reverse(totalPath.begin(), totalPath.end());
    return totalPath;
}

float Pathfinder::heuristic(const Vector2& a, const Vector2& b) const {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y); // Manhattan distance
}