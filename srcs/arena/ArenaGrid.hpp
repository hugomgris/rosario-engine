#pragma once

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <raylib.h>
#include "../../incs/DataStructs.hpp"

enum class CellType {
	Empty,
	Wall,
	Obstacle,
	SpawningSolid,
	DespawningSolid
};

enum class WallPreset {
	InterLock1,
	Spiral1,
	Columns1,
	Columns2,
	Cross,
	Checkerboard,
	Maze,
	Diamond,
	Tunnels,
	FourRooms
};

class ArenaGrid {
	private:
		std::vector<std::vector<CellType>> _grid;
		
		int _gridWidth;
		int _gridHeight;

		float _spawnTimer		= 0.0f;
		float _spawnDuration	= 0.0f;
		float _fadeTimer		= 0.0f;
		float _fadeDuration		= 0.1f;

		float _despawnTimer		= 0.0f;
		float _fadeOutTimer		= 0.0f;
		float _fadeOutDuration	= 0.15f;

	public:
		ArenaGrid(int width, int height);

		void		setCell(int x, int y, CellType type);
		CellType	getCell(int x, int y) const;
		bool		isWalkable(int x, int y) const;

		std::vector<Vec2> getAvailableCells() const;
		std::vector<std::vector<CellType>> getGrid() const;

		void spawnObstacle(int x, int y, int w, int h);

		void growWall(int x, int y, int w, int h);

		void clearCell(int x, int y);
		void clearArena();

		void transformArenaWithPreset(WallPreset preset);

		std::vector<std::vector<Vector2>> getAllOutlines(int offsetX, int offsetY) const;

		int getPlayWidth()	const { return _gridWidth - 2; }
		int getPlayHeight()	const { return _gridHeight - 2; }

		int getFullWidth()	const { return _gridWidth; }
		int getFullHeight()	const { return _gridHeight; }

		void setMenuArena();

		void	beginSpawn(float solidifyDelay);
		void	tickSpawnTimer(float dt);
		bool	isSpawning() const;
		float	getSpawnFadeProgress() const; // 0-1

		void	beginDespawn(float delay);
		void	tickDespawnTimer(float dt);
		bool	isDespawning() const;
		float	getDespawnFadeProgress() const; // 0-1
};