#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "AI/GridHelper.hpp"
#include "systems/AISystem.hpp"

// 1 - GridHelper: Grid coordinate validation (bounds checking)
TEST(GridHelper, GridCoordinateValidation) {
	Registry registry;
	//ArenaGrid* arena = new ArenaGrid(32, 32);
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	GridHelper gridHelper;
	AISystem aiSystem(32, 32);

	const BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	const std::vector<Vec2> ignorePositions = {};

	Vec2 p1 = {5, 5};
	bool b1 = gridHelper.isWalkable(blocked, p1, 10, 10, ignorePositions);
	EXPECT_TRUE(b1);

	Vec2 p2 = {-1, -1};
	bool b2 = gridHelper.isWalkable(blocked, p2, 10, 10, ignorePositions);
	EXPECT_TRUE(b2 == false);

	Vec2 p3 = {-1, 5};
	bool b3 = gridHelper.isWalkable(blocked, p3, 10, 10, ignorePositions);
	EXPECT_TRUE(b3 == false);

	Vec2 p4 = {5, 11};
	bool b4 = gridHelper.isWalkable(blocked, p4, 10, 10, ignorePositions);
	EXPECT_TRUE(b4 == false);

	Vec2 p5 = {9, 9};
	bool b5 = gridHelper.isWalkable(blocked, p5, 10, 10, ignorePositions);
	EXPECT_TRUE(b5);
}

// 2 - GridHelper: Neighbor generation for interior cell
TEST(GridHelper, NeighbourGenerationFromInteriorCell) {
	GridHelper gridHelper;

	Vec2 p1 = {4, 4};
	std::vector<Vec2> n1 = gridHelper.getNeighbors(p1, 32, 32);

	EXPECT_EQ(n1.size(), 4);
	EXPECT_EQ(n1[0].x, 3);
	EXPECT_EQ(n1[0].y, 4);
	EXPECT_EQ(n1[1].x, 5);
	EXPECT_EQ(n1[1].y, 4);
	EXPECT_EQ(n1[2].x, 4);
	EXPECT_EQ(n1[2].y, 3);
	EXPECT_EQ(n1[3].x, 4);
	EXPECT_EQ(n1[3].y, 5);
}

// 3 - GridHelper: Neighbor generation at grid edges
TEST(GridHelper, NeighbourGenerationAtGridEdges) {
	GridHelper gridHelper;

	Vec2 p1 = {4, 0};
	std::vector<Vec2> n1 = gridHelper.getNeighbors(p1, 10, 10);

	EXPECT_EQ(n1.size(), 3);
	EXPECT_EQ(n1[0].x, 3);
	EXPECT_EQ(n1[0].y, 0);
	EXPECT_EQ(n1[1].x, 5);
	EXPECT_EQ(n1[1].y, 0);
	EXPECT_EQ(n1[2].x, 4);
	EXPECT_EQ(n1[2].y, 1);
}

// 4 - GridHelper: Neighbor generation at grid corners
TEST(GridHelper, NeighbourGenerationAtCorners) {
	GridHelper gridHelper;

	Vec2 p1 = {0, 0};
	std::vector<Vec2> n1 = gridHelper.getNeighbors(p1, 10, 10);

	EXPECT_EQ(n1.size(), 2);
	EXPECT_EQ(n1[0].x, 1);
	EXPECT_EQ(n1[0].y, 0);
	EXPECT_EQ(n1[1].x, 0);
	EXPECT_EQ(n1[1].y, 1);
}

// 5 - GridHelper: Manhattan distance calculation
TEST(GridHelper, ManhattanDistanceCalculation) {
	GridHelper gridHelper;

	Vec2 a1 = {2, 7};
	Vec2 b1 = {8, 1};
	int r1 = gridHelper.manhattanDistance(a1, b1);
	EXPECT_EQ(r1, 12);

	Vec2 a2 = {15, 15};
	Vec2 b2 = {0, 0};
	int r2 = gridHelper.manhattanDistance(a2, b2);
	EXPECT_EQ(r2, 30);
}