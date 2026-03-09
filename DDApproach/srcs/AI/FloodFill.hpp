#ifndef FLOODFILL_HPP
#define FLOODFILL_HPP

#include <vector>
#include "GridHelper.hpp"

class FloodFill {
public:
    FloodFill(int width, int height);
    void performFill(int startX, int startY, CellType targetType, CellType replacementType);
    std::vector<std::pair<int, int>> getFilledCells() const;

private:
    int width;
    int height;
    std::vector<std::vector<CellType>> grid;
    std::vector<std::pair<int, int>> filledCells;

    void flood(int x, int y, CellType targetType, CellType replacementType);
};

#endif // FLOODFILL_HPP