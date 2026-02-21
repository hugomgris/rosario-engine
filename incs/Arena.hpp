#pragma once
#include "Renderer.hpp"
#include <vector>
#include <map>
#include <set>

enum class CellType {
	Empty,
	Wall,
	Obstacle,
	Snake_A,
	Snake_B,
	Food
};

enum class WallPreset {
	InterLock1,
	Spiral1
};

class Arena {
	private:
		std::vector<std::vector<CellType>> grid;
		int gridWidth;
		int gridHeight;
		int squareSize;

		Vector2 foodPosition;

	public:
		Arena(int width, int height, int squareSize); // W and H are in amount-of-squares magnitude
		~Arena() = default;

		// Grid manipulation (uses game coordinates: 0 to width-1, 0 to height-1)
		std::vector<std::vector<CellType>> getGrid() const;
		void setCell(int x, int y, CellType type);
		CellType getCell(int x, int y) const;
		bool isWalkable(int x, int y) const;
		void setFoodCell(int x, int y);
		Vector2 getFoodPosition() const;
		const std::vector<Vec2> getAvailableCells() const;

		// obstacle management
		void spawnObstacle(int x, int y, int width, int height);
		void transformArenaWithPreset(WallPreset preset);
		void growWall(int x, int y, int width, int height);
		void clearCell(int x, int y);
		void clearArena();

		// outline extraction for tunnel lines
		std::vector<Vector2> getArenaOutline(int offsetX, int offsetY);

		// rendering
		void render(const Renderer& renderer) const;
};