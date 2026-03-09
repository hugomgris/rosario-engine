#include "FloodFill.hpp"
#include "GridHelper.hpp"
#include <vector>
#include <queue>

FloodFill::FloodFill(int width, int height) : width(width), height(height) {
    grid.resize(height, std::vector<int>(width, 0));
}

void FloodFill::fill(int startX, int startY, int fillValue) {
    if (startX < 0 || startX >= width || startY < 0 || startY >= height) {
        return; // Out of bounds
    }

    int targetValue = grid[startY][startX];
    if (targetValue == fillValue) {
        return; // Already filled
    }

    std::queue<std::pair<int, int>> toFill;
    toFill.push({startX, startY});

    while (!toFill.empty()) {
        auto [x, y] = toFill.front();
        toFill.pop();

        if (x < 0 || x >= width || y < 0 || y >= height || grid[y][x] != targetValue) {
            continue; // Out of bounds or not the target value
        }

        grid[y][x] = fillValue; // Fill the cell

        // Add neighboring cells to the queue
        toFill.push({x + 1, y});
        toFill.push({x - 1, y});
        toFill.push({x, y + 1});
        toFill.push({x, y - 1});
    }
}

const std::vector<std::vector<int>>& FloodFill::getGrid() const {
    return grid;
}