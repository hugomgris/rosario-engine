#ifndef GRIDHELPER_HPP
#define GRIDHELPER_HPP

#include <vector>
#include "AIConfig.hpp"
#include "FloodFill.hpp"

class GridHelper {
public:
    static std::vector<std::pair<int, int>> getAvailableCells(const std::vector<std::vector<CellType>>& grid);
    static bool isCellWalkable(int x, int y, const std::vector<std::vector<CellType>>& grid);
    static std::vector<std::pair<int, int>> floodFill(int startX, int startY, const std::vector<std::vector<CellType>>& grid);
    static std::vector<std::pair<int, int>> findPath(int startX, int startY, int targetX, int targetY, const std::vector<std::vector<CellType>>& grid);
};

#endif // GRIDHELPER_HPP