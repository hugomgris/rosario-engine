#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "AI/Pathfinder.hpp"
#include "AI/AIPresetLoader.hpp"
#include "systems/AISystem.hpp"
#include "helpers/Factories.hpp"

// 1 - Pathfinder: Find path on empty grid (straight line)
TEST(Pathfinder, FindPathOnEmptyGrid) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {31,0}, 32, 32, 50, ignored);
	EXPECT_EQ(path.size(), 31);

	std::vector<Vec2> path2 = pathfinder.findPath(blocked, {15,15}, {15,20}, 32, 32, 50, ignored);
	EXPECT_EQ(path2.size(), 5);
}

// 2 - Pathfinder: Find path with single obstacle (circumnavigate)
TEST(Pathfinder, FindPathWithObstacle) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 10, 1);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 300, ignored);
	EXPECT_EQ(path.size(), 51);
}

// 3 - Pathfinder: Find path with complex obstacle maze
TEST(Pathfinder, FindPathWithComplexArena) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 31, 1);
	arena->spawnObstacle(25, 15, 1, 10);
	arena->spawnObstacle(15, 17, 1, 21);
	arena->spawnObstacle(10, 15, 1, 10);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	// With a very complex configuration, the maxDepth needs to be quite high,
	// higher than what I have in any of my AI configurations (json)
	// Nevertheless, the point here is to prove that a path *can* be found
	// Also: very complex = the snake is going to have to travel a very long distance to reach the target
	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 1400, ignored);
	EXPECT_TRUE(path.size() > 0);
}

// 4 - Pathfinder: No path returns empty vector
TEST(Pathfinder, NoPathReturnsEmptyVector) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 32, 1);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	// With a very complex configuration, the maxDepth needs to be quite high,
	// higher than what I have in any of my AI configurations (json)
	// Nevertheless, the point here is to prove that a path *can* be found
	// Also: very complex = the snake is going to have to travel a very long distance to reach the target
	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 300, ignored);
	EXPECT_TRUE(path.size() == 0);
}

// 5 - Pathfinder: Respects maxDepth limit
// Reusing test 3 configuration, just with a lower maxDepth value
TEST(Pathfinder, PathReturnsEmptyVector_WhenMaxDepthIsNotEnough) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 31, 1);
	arena->spawnObstacle(25, 15, 1, 10);
	arena->spawnObstacle(15, 17, 1, 21);
	arena->spawnObstacle(10, 15, 1, 10);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	// With a maxDepth of 300 (preset DIFFICULT), a path cannot be found in this configuration
	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 300, ignored);
	EXPECT_TRUE(path.size() == 0);
}

// 6 - Pathfinder: Path is shortest (or near-optimal with A*)
TEST(Pathfinder, PathIsShortestOrNearOptimal) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 31, 1);
	arena->spawnObstacle(25, 15, 1, 10);
	arena->spawnObstacle(15, 17, 1, 21);
	arena->spawnObstacle(10, 15, 1, 10);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {};

	// With this arena configuration, the shortest path has 111 steps
	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 1500, ignored);
	EXPECT_EQ(path.size(), 111);
}

// 7 - Pathfinder: Properly ignores specified positions
TEST(Pathfinder, ProperlyIgnoresSpecifiedPositions) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	arena->spawnObstacle(0, 15, 32, 1);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {{0,15}};

	// With this arena configuration, the shortest path has 111 steps
	std::vector<Vec2> path = pathfinder.findPath(blocked, {0,0}, {0,31}, 32, 32, 1500, ignored);
	EXPECT_EQ(path.size(), 31);
}

// 8 - Pathfinder: Start equals goal returns path of length 0
TEST(Pathfinder, PathIsLengthZero_WhenStartEqualsGoal) {
	Registry registry;
	Pathfinder pathfinder;
	std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
	AISystem aiSystem(32, 32);
	BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());
	std::vector<Vec2> ignored = {{0,15}};

	// With this arena configuration, the shortest path has 111 steps
	std::vector<Vec2> path = pathfinder.findPath(blocked, {15,15}, {15,15}, 32, 32, 1500, ignored);
	EXPECT_EQ(path.size(), 0);
}

// 9 - Pathfinder: No backtracking in returned path
TEST(Pathfinder, ReturnedPathHasNoBacktrackingOrLoops) {
    Registry registry;
    Pathfinder pathfinder;
    std::unique_ptr<ArenaGrid> arena = std::make_unique<ArenaGrid>(32, 32);
    arena->spawnObstacle(0, 15, 31, 1);
    arena->spawnObstacle(25, 15, 1, 10);
    arena->spawnObstacle(15, 17, 1, 21);
    arena->spawnObstacle(10, 15, 1, 10);
    AISystem aiSystem(32, 32);
    BlockedGrid blocked = aiSystem.buildBlockedGrid(registry, arena.get());

    const Vec2 start = {0, 0};
    const Vec2 goal  = {0, 31};
    std::vector<Vec2> ignored = {};

    std::vector<Vec2> path = pathfinder.findPath(blocked, start, goal, 32, 32, 1500, ignored);
    ASSERT_FALSE(path.empty());

    auto isAdjacent = [](Vec2 a, Vec2 b) {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        return (dx + dy) == 1;
    };

    // Path is returned without start included, so first step must be adjacent to start.
    EXPECT_TRUE(isAdjacent(start, path.front()));

    // Cardinal adjacency all along.
    for (size_t i = 1; i < path.size(); ++i) {
        EXPECT_TRUE(isAdjacent(path[i - 1], path[i]));
    }

    // No immediate reversal: A -> B -> A
    for (size_t i = 2; i < path.size(); ++i) {
        bool immediateBacktrack =
            (path[i - 2].x == path[i].x) && (path[i - 2].y == path[i].y);
        EXPECT_FALSE(immediateBacktrack);
    }

    // No repeated positions anywhere in path (strongest anti-loop check).
    std::set<std::pair<int, int>> seen;
    for (const Vec2& p : path) {
        auto inserted = seen.insert({p.x, p.y});
        EXPECT_TRUE(inserted.second);
    }

    // Last node should be goal.
    EXPECT_EQ(path.back().x, goal.x);
    EXPECT_EQ(path.back().y, goal.y);
}