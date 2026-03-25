#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "AI/FloodFill.hpp"
#include "AI/AIPresetLoader.hpp"
#include "systems/AISystem.hpp"
#include "helpers/Factories.hpp"

// 1 - FloodFill: Count reachable cells on empty grid
TEST(FloodFill, CountReachableCellsInEmptyGrid) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};
	
	Vec2 start = {0,0};
	int reachable = floodFill.countReachable(blocked, start, 32, 32, ignored);
	EXPECT_EQ(reachable, (32*32));
}

// 2 - FloodFill: Count reachable cells with obstacles
// 3 - FloodFill: Count reachable from various start positions
// two for one because setting up the context for 3, which will be the same as for 2, is wasting time
TEST(FloodFill, CountReachableCellsWithObstacles) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(5, 5, 5, 5);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};
	
	Vec2 s1 = {0,0};
	Vec2 s2 = {16,28};
	Vec2 s3 = {31,31};
	int r1 = floodFill.countReachable(blocked, s1, 32, 32, ignored);
	int r2 = floodFill.countReachable(blocked, s2, 32, 32, ignored);
	int r3 = floodFill.countReachable(blocked, s3, 32, 32, ignored);
	EXPECT_EQ(r1, (32*32) - 25);
	EXPECT_EQ(r2, (32*32) - 25);
	EXPECT_EQ(r3, (32*32) - 25);
}

// 4 - FloodFill: Properly ignores specified positions
TEST(FloodFill, CountReachableIgnoresSpecifiedPositions) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(5, 5, 2, 2);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {{5,5}, {6,5}, {5,6}, {6,6}};
	
	Vec2 s1 = {0,0};
	Vec2 s2 = {16,28};
	Vec2 s3 = {31,31};
	int r1WithoutIgnore = floodFill.countReachable(blocked, s1, 32, 32, {});
	int r2WithoutIgnore = floodFill.countReachable(blocked, s2, 32, 32, {});
	int r3WithoutIgnore = floodFill.countReachable(blocked, s3, 32, 32, {});
	int r1WithIgnore = floodFill.countReachable(blocked, s1, 32, 32, ignored);
	int r2WithIgnore = floodFill.countReachable(blocked, s2, 32, 32, ignored);
	int r3WithIgnore = floodFill.countReachable(blocked, s3, 32, 32, ignored);

	EXPECT_EQ(r1WithoutIgnore, (32*32) - 4);
	EXPECT_EQ(r2WithoutIgnore, (32*32) - 4);
	EXPECT_EQ(r3WithoutIgnore, (32*32) - 4);
	EXPECT_EQ(r1WithIgnore, (32*32));
	EXPECT_EQ(r2WithIgnore, (32*32));
	EXPECT_EQ(r3WithIgnore, (32*32));
}

// 5 - FloodFill: Detect unreachable areas (isolated regions)
TEST(FloodFill, DetectUnreachableAreas) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(21, 21, 11, 1);
	arena->spawnObstacle(21, 21, 1, 11);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());

	Vec2 s1 = {4, 4};
	int r1 = floodFill.countReachable(blocked, s1, 32, 32, {});
	const int enclosedRegion = 10 * 10;
	const int obstacleTiles = 11 + 11 - 1;
	EXPECT_EQ(r1, (32 * 32) - enclosedRegion - obstacleTiles);
}

// 6 - FloodFill: Single cell reachable when surrounded (but not blocked itself)
TEST(FloodFill, SingleCell_WhenCompletelySurrounded) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(14, 14, 1, 1);
	arena->spawnObstacle(15, 13, 1, 1);
	arena->spawnObstacle(15, 15, 1, 1);
	arena->spawnObstacle(16, 14, 1, 1);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());

	Vec2 s1 = {15,14};
	int r1 = floodFill.countReachable(blocked, s1, 32, 32, {});
	EXPECT_EQ(r1, 1);
}

// 7 - FloodFill: canReachTail() returns true when tail reachable
TEST(FloodFill, CanReachTailReturnsTrue_WhenTailIsRechable) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());

	AIPresetLoader::PresetTable preset;
	EXPECT_NO_THROW(preset = AIPresetLoader::load("data/AIPresets.json"));

	EXPECT_EQ(preset.size(), 3);

	Entity aiSnake = Factories::spawnAISnake(registry, {10,10}, 4, {0,0,0,0}, "medium", preset);
	std::vector<Vec2> path = {{10,11}, {10,12}, {10,13}, {10, 14}};

	bool b1 = floodFill.canReachTail(registry, blocked, aiSnake, path, 32, 32);
	EXPECT_TRUE(b1);
}

// 8 - FloodFill: canReachTail() returns false when tail blocked
TEST(FloodFill, CanReachTailReturnsFalse_WhenTailBlocked) {
	Registry registry;
	FloodFill floodFill;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(4, 0, 32, 1);
	arena->spawnObstacle(0, 1, 32, 32);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());

	AIPresetLoader::PresetTable preset;
	EXPECT_NO_THROW(preset = AIPresetLoader::load("data/AIPresets.json"));

	EXPECT_EQ(preset.size(), 3);

	Entity aiSnake = Factories::spawnAISnake(registry, {3,0}, 4, {0,0,0,0}, "medium", preset);
	std::vector<Vec2> path = {{10,11}, {10,12}, {10,13}, {10, 14}};

	bool b1 = floodFill.canReachTail(registry, blocked, aiSnake, path, 32, 32);
	EXPECT_FALSE(b1);
}