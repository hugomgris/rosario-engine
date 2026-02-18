#include "../../incs/Arena.hpp"

Arena::Arena(int width, int height, int squareSize):
		gridWidth(width + 2), gridHeight(height + 2), squareSize(squareSize), foodPosition(Vector2{0,0}) {
	clearArena();
}

// grid manipulation
void Arena::setCell(int x, int y, CellType type) { 
	grid[y + 1][x + 1] = type; 
}

CellType Arena::getCell(int x, int y) const { 
	// Bounds check for game coordinates
	if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2) {
		return CellType::Wall;  // Out of bounds treated as wall
	}
	return grid[y + 1][x + 1]; 
}

bool Arena::isWalkable(int x, int y) const {
	// Bounds check
	if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2) {
		return false;
	}
	
	CellType target = grid[y + 1][x + 1];

	if (target != CellType::Empty)
		return false;

	return true;
}

void Arena::setFoodCell(int x, int y) { 
	foodPosition.x = x;
	foodPosition.y = y;
	setCell(x, y, CellType::Food);

	//DEBUG output
	/* std::cout << "Food set at (" << x << ", " << y << ")" << std::endl;

	int foodAmount = 0;
	for (int i = 0; i < gridHeight; i++) {
		for (int j = 0; j < gridWidth; j++) {
			if (grid[i][j] == CellType::Food) {
				foodAmount++;
				std::cout << "Food found at grid[" << i << "][" << j << "]" << std::endl;
			}
		}
	}

	std::cout << "Total food in arena: " << foodAmount << std::endl; */
}

Vector2 Arena::getFoodPosition() const {
	return foodPosition;
}

//obstacle management
void Arena::spawnObstacle(int x, int y, int width, int height) {
	// Translate to arena grid coordinates and use setCell for consistency
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			setCell(x + i, y + j, CellType::Obstacle);
		}
	}
}

// works by: targeting a WALL TYPE node, checking the direction of growth via width/height sign
void Arena::growWall(int x, int y, int width, int height) {
	// Check if starting position is valid in game coordinates
	if (x < -1 || x >= gridWidth - 1 || y < -1 || y >= gridHeight - 1) {
		std::cout << "error: wall growth: anchor out of bounds!" << std::endl;
		return;
	}

	if (getCell(x, y) != CellType::Wall) {
		std::cout << "error: wall growth: base node was not a wall!" << std::endl;
		return;
	}

	int xStep = (width >= 0) ? 1 : -1;
	int yStep = (height >= 0) ? 1 : -1;

	// Grow from the anchor position
	for (int i = x; i != x + width + xStep; i += xStep) {
		if (i < -1 || i >= gridWidth - 1) continue;
		for (int j = y; j != y + height + yStep; j += yStep) {
			if (j < -1 || j >= gridHeight - 1) continue;
			setCell(i, j, CellType::Wall);
		}
	}
}

void Arena::clearCell(int x, int y) { 
	grid[y + 1][x + 1] = CellType::Empty; 
}

void Arena::clearArena() {
	grid.assign(gridHeight, std::vector<CellType>(gridWidth, CellType::Empty));

	// Set up border walls
	for (int y = 0; y < gridHeight; y++) {
		for (int x = 0; x < gridWidth; x++) {
			if (x == 0 || x == gridWidth - 1 || y == 0 || y == gridHeight - 1)
				grid[y][x] = CellType::Wall;
			else
				grid[y][x] = CellType::Empty;
		}
	}
}

void Arena::render(const Renderer& renderer) const {
	for (int y = 0; y < gridHeight; y++) {
		for (int x = 0; x < gridWidth; x++) {
			if (grid[y][x] == CellType::Empty) continue;
			if (grid[y][x] == CellType::Wall || grid[y][x] == CellType::Obstacle) {
				// Arena grid coordinates directly match render coordinates
				// No translation needed here
				renderer.drawBorderBrick(x, y, wallColor);
			}
		}
	}
}