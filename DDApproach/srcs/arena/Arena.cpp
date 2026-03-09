#include "Arena.hpp"
#include "ArenaConfig.hpp"
#include "ecs/Registry.hpp"
#include <fstream>
#include <iostream>

Arena::Arena(const ArenaConfig& config) 
    : gridWidth(config.width), gridHeight(config.height), squareSize(config.squareSize) {
    grid.resize(gridHeight, std::vector<CellType>(gridWidth, CellType::Empty));
    initializeWalls();
}

void Arena::initializeWalls() {
    for (int x = 0; x < gridWidth; ++x) {
        setCell(x, 0, CellType::Wall);
        setCell(x, gridHeight - 1, CellType::Wall);
    }
    for (int y = 0; y < gridHeight; ++y) {
        setCell(0, y, CellType::Wall);
        setCell(gridWidth - 1, y, CellType::Wall);
    }
}

void Arena::setCell(int x, int y, CellType type) {
    if (isValidCoordinate(x, y)) {
        grid[y][x] = type;
    }
}

CellType Arena::getCell(int x, int y) const {
    if (isValidCoordinate(x, y)) {
        return grid[y][x];
    }
    return CellType::Wall; // Out of bounds returns wall
}

bool Arena::isWalkable(int x, int y) const {
    return isValidCoordinate(x, y) && grid[y][x] == CellType::Empty;
}

bool Arena::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < gridWidth && y >= 0 && y < gridHeight;
}

void Arena::render(const Renderer& renderer) const {
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            if (grid[y][x] == CellType::Wall) {
                renderer.drawSquare(x * squareSize, y * squareSize, squareSize, Color::WHITE);
            }
        }
    }
}

void Arena::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open arena config file: " << filename << std::endl;
        return;
    }
    ArenaConfig config;
    file >> config; // Assuming operator>> is overloaded for ArenaConfig
    *this = Arena(config);
}

std::vector<Vector2> Arena::getAvailableCells() const {
    std::vector<Vector2> availableCells;
    for (int y = 1; y < gridHeight - 1; ++y) {
        for (int x = 1; x < gridWidth - 1; ++x) {
            if (grid[y][x] == CellType::Empty) {
                availableCells.emplace_back(x, y);
            }
        }
    }
    return availableCells;
}

void Arena::clearArena() {
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            grid[y][x] = CellType::Empty;
        }
    }
    initializeWalls();
}