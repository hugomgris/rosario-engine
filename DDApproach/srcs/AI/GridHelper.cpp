#include "GridHelper.hpp"
#include "AIConfig.hpp"
#include <vector>
#include <algorithm>

class GridHelper {
public:
    static std::vector<Vector2> getAvailableCells(const std::vector<std::vector<CellType>>& grid) {
        std::vector<Vector2> availableCells;
        for (size_t y = 0; y < grid.size(); ++y) {
            for (size_t x = 0; x < grid[y].size(); ++x) {
                if (grid[y][x] == CellType::Empty) {
                    availableCells.emplace_back(Vector2{x, y});
                }
            }
        }
        return availableCells;
    }

    static bool isCellWalkable(const std::vector<std::vector<CellType>>& grid, int x, int y) {
        if (y < 0 || y >= grid.size() || x < 0 || x >= grid[y].size()) {
            return false;
        }
        return grid[y][x] == CellType::Empty;
    }

    static void markCellAsOccupied(std::vector<std::vector<CellType>>& grid, int x, int y) {
        if (isCellWalkable(grid, x, y)) {
            grid[y][x] = CellType::Obstacle;
        }
    }

    static void clearCell(std::vector<std::vector<CellType>>& grid, int x, int y) {
        if (y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
            grid[y][x] = CellType::Empty;
        }
    }
};