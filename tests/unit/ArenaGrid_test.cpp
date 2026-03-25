#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresetLoader.hpp"

namespace {
const ArenaPresetDefinition& findPresetByName(const ArenaPresetLoader::PresetList& presets,
											  const std::string& name) {
	for (const auto& preset : presets) {
		if (preset.name == name) {
			return preset;
		}
	}
	throw std::runtime_error("ArenaGrid_test: preset not found: " + name);
}

int countMaskWalls(const std::vector<std::string>& mask) {
	int count = 0;
	for (const auto& row : mask) {
		for (char c : row) {
			if (c == '#') {
				++count;
			}
		}
	}
	return count;
}

int countCellsByType(const ArenaGrid& arena, CellType type) {
	int count = 0;
	for (int y = 0; y < arena.getPlayHeight(); ++y) {
		for (int x = 0; x < arena.getPlayWidth(); ++x) {
			if (arena.getCell(x, y) == type) {
				++count;
			}
		}
	}
	return count;
}
}

TEST(ArenaGrid, ConstructorInitializesExpectedDimensionsAndPerimeterWalls) {
	ArenaGrid arena(32, 32);

	EXPECT_EQ(arena.getPlayWidth(), 32);
	EXPECT_EQ(arena.getPlayHeight(), 32);
	EXPECT_EQ(arena.getFullWidth(), 34);
	EXPECT_EQ(arena.getFullHeight(), 34);

	const auto grid = arena.getGrid();
	for (int x = 0; x < arena.getFullWidth(); ++x) {
		EXPECT_EQ(grid[0][x], CellType::Wall);
		EXPECT_EQ(grid[arena.getFullHeight() - 1][x], CellType::Wall);
	}
	for (int y = 0; y < arena.getFullHeight(); ++y) {
		EXPECT_EQ(grid[y][0], CellType::Wall);
		EXPECT_EQ(grid[y][arena.getFullWidth() - 1], CellType::Wall);
	}
}

TEST(ArenaGrid, SetCellAndGetCellBasicOperations) {
	ArenaGrid arena(8, 8);
	arena.setCell(3, 4, CellType::Obstacle);
	EXPECT_EQ(arena.getCell(3, 4), CellType::Obstacle);

	arena.setCell(3, 4, CellType::Wall);
	EXPECT_EQ(arena.getCell(3, 4), CellType::Wall);
}

TEST(ArenaGrid, GetCellOutOfBoundsReturnsWall) {
	ArenaGrid arena(8, 8);
	EXPECT_EQ(arena.getCell(-1, 0), CellType::Wall);
	EXPECT_EQ(arena.getCell(0, -1), CellType::Wall);
	EXPECT_EQ(arena.getCell(8, 0), CellType::Wall);
	EXPECT_EQ(arena.getCell(0, 8), CellType::Wall);
}

TEST(ArenaGrid, IsWalkableReflectsCellTypeAndBounds) {
	ArenaGrid arena(8, 8);
	EXPECT_TRUE(arena.isWalkable(0, 0));

	arena.setCell(1, 1, CellType::Obstacle);
	EXPECT_FALSE(arena.isWalkable(1, 1));

	arena.setCell(2, 2, CellType::Wall);
	EXPECT_FALSE(arena.isWalkable(2, 2));

	arena.setCell(3, 3, CellType::SpawningSolid);
	EXPECT_FALSE(arena.isWalkable(3, 3));

	EXPECT_FALSE(arena.isWalkable(-1, 0));
	EXPECT_FALSE(arena.isWalkable(0, -1));
	EXPECT_FALSE(arena.isWalkable(8, 0));
	EXPECT_FALSE(arena.isWalkable(0, 8));
}

TEST(ArenaGrid, GetAvailableCellsReturnsOnlyEmptyCellsAndExpectedCount) {
	ArenaGrid arena(4, 4);
	EXPECT_EQ(static_cast<int>(arena.getAvailableCells().size()), 16);

	arena.spawnObstacle(1, 1, 2, 2);
	auto freeCells = arena.getAvailableCells();
	EXPECT_EQ(static_cast<int>(freeCells.size()), 12);

	for (const Vec2& cell : freeCells) {
		EXPECT_EQ(arena.getCell(cell.x, cell.y), CellType::Empty);
		EXPECT_TRUE(arena.isWalkable(cell.x, cell.y));
	}
}

TEST(ArenaGrid, ClearCellSetsCellToEmpty) {
	ArenaGrid arena(8, 8);
	arena.setCell(2, 2, CellType::Obstacle);
	ASSERT_EQ(arena.getCell(2, 2), CellType::Obstacle);

	arena.clearCell(2, 2);
	EXPECT_EQ(arena.getCell(2, 2), CellType::Empty);
}

TEST(ArenaGrid, ClearArenaResetsInteriorToEmptyAndPreservesPerimeterWalls) {
	ArenaGrid arena(8, 8);
	arena.spawnObstacle(0, 0, 8, 1);
	arena.spawnObstacle(2, 2, 3, 3);

	arena.clearArena();

	EXPECT_EQ(countCellsByType(arena, CellType::Obstacle), 0);
	EXPECT_EQ(static_cast<int>(arena.getAvailableCells().size()), 64);

	const auto grid = arena.getGrid();
	for (int x = 0; x < arena.getFullWidth(); ++x) {
		EXPECT_EQ(grid[0][x], CellType::Wall);
		EXPECT_EQ(grid[arena.getFullHeight() - 1][x], CellType::Wall);
	}
}

TEST(ArenaGrid, SpawnObstacleCreatesRectangularObstacle) {
	ArenaGrid arena(10, 10);
	arena.spawnObstacle(2, 3, 4, 2);

	for (int y = 3; y < 5; ++y) {
		for (int x = 2; x < 6; ++x) {
			EXPECT_EQ(arena.getCell(x, y), CellType::Obstacle);
		}
	}
}

TEST(ArenaGrid, GrowWallCanExtendFromPerimeterIntoPlayfield) {
	ArenaGrid arena(8, 8);

	// Anchor on top wall border and grow downward two cells.
	arena.growWall(0, -1, 0, 2);

	EXPECT_EQ(arena.getCell(0, 0), CellType::Wall);
	EXPECT_EQ(arena.getCell(0, 1), CellType::Wall);
}

TEST(ArenaGrid, TransformArenaWithPresetInterLock1MatchesWallMaskCount) {
	ArenaGrid arena(32, 32);
	auto presets = ArenaPresetLoader::load("data/ArenaPresets.json");
	const auto& preset = findPresetByName(presets, "InterLock1");

	arena.transformArenaWithPreset(preset.walls);
	EXPECT_EQ(countCellsByType(arena, CellType::SpawningSolid), countMaskWalls(preset.walls));
}

TEST(ArenaGrid, TransformArenaWithPresetSpiral1MatchesWallMaskCount) {
	ArenaGrid arena(32, 32);
	auto presets = ArenaPresetLoader::load("data/ArenaPresets.json");
	const auto& preset = findPresetByName(presets, "Spiral1");

	arena.transformArenaWithPreset(preset.walls);
	EXPECT_EQ(countCellsByType(arena, CellType::SpawningSolid), countMaskWalls(preset.walls));
}

TEST(ArenaGrid, TransformArenaWithPresetColumns1MatchesWallMaskCount) {
	ArenaGrid arena(32, 32);
	auto presets = ArenaPresetLoader::load("data/ArenaPresets.json");
	const auto& preset = findPresetByName(presets, "Columns1");

	arena.transformArenaWithPreset(preset.walls);
	EXPECT_EQ(countCellsByType(arena, CellType::SpawningSolid), countMaskWalls(preset.walls));
}

TEST(ArenaGrid, TransformArenaWithPresetMazeMatchesWallMaskCount) {
	ArenaGrid arena(32, 32);
	auto presets = ArenaPresetLoader::load("data/ArenaPresets.json");
	const auto& preset = findPresetByName(presets, "Maze");

	arena.transformArenaWithPreset(preset.walls);
	EXPECT_EQ(countCellsByType(arena, CellType::SpawningSolid), countMaskWalls(preset.walls));
}

TEST(ArenaGrid, GetAllOutlinesReturnsGeometryForSolids) {
	ArenaGrid arena(16, 16);
	const auto baseline = arena.getAllOutlines(0, 0);

	arena.spawnObstacle(4, 4, 4, 4);
	const auto withObstacle = arena.getAllOutlines(0, 0);

	EXPECT_FALSE(withObstacle.empty());
	EXPECT_GE(withObstacle.size(), baseline.size());
	for (const auto& poly : withObstacle) {
		EXPECT_FALSE(poly.empty());
	}
}

TEST(ArenaGrid, BeginSpawnAndTickSpawnTimerSolidifiesSpawningCells) {
	ArenaGrid arena(8, 8);
	arena.setCell(2, 2, CellType::SpawningSolid);

	arena.beginSpawn(0.5f);
	EXPECT_TRUE(arena.isSpawning());

	arena.tickSpawnTimer(0.6f);
	EXPECT_EQ(arena.getCell(2, 2), CellType::Obstacle);
	EXPECT_TRUE(arena.isSpawning());

	arena.tickSpawnTimer(1.0f);
	EXPECT_FALSE(arena.isSpawning());
}

TEST(ArenaGrid, SetMenuArenaAndSetGameplayArenaSwitchExpectedDimensions) {
	ArenaGrid arena(32, 32);
	EXPECT_EQ(arena.getPlayWidth(), 32);
	EXPECT_EQ(arena.getPlayHeight(), 32);

	arena.setMenuArena();
	EXPECT_EQ(arena.getPlayWidth(), 60);
	EXPECT_EQ(arena.getPlayHeight(), 33);

	arena.setGameplayArena();
	EXPECT_EQ(arena.getPlayWidth(), 32);
	EXPECT_EQ(arena.getPlayHeight(), 32);
}

TEST(ArenaGrid, PerimeterWallsRemainIntactAfterPresetTransform) {
	ArenaGrid arena(32, 32);
	auto presets = ArenaPresetLoader::load("data/ArenaPresets.json");
	const auto& preset = findPresetByName(presets, "InterLock1");
	arena.transformArenaWithPreset(preset.walls);

	const auto grid = arena.getGrid();
	for (int x = 0; x < arena.getFullWidth(); ++x) {
		EXPECT_EQ(grid[0][x], CellType::Wall);
		EXPECT_EQ(grid[arena.getFullHeight() - 1][x], CellType::Wall);
	}
	for (int y = 0; y < arena.getFullHeight(); ++y) {
		EXPECT_EQ(grid[y][0], CellType::Wall);
		EXPECT_EQ(grid[y][arena.getFullWidth() - 1], CellType::Wall);
	}
}
