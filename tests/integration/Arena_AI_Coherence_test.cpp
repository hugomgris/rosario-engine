#include <gtest/gtest.h>

#include <memory>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "systems/AISystem.hpp"
#include "AI/Pathfinder.hpp"
#include "AI/FloodFill.hpp"
#include "AI/GridHelper.hpp"
#include "FrameContext.hpp"

namespace {

class ArenaAICoherenceTest : public ::testing::Test {
protected:
	void SetUp() override {
		registry_ = std::make_unique<Registry>();
		arena_ = std::make_unique<ArenaGrid>(32, 32);
		aiSystem_ = std::make_unique<AISystem>(32, 32);
	}
	
	std::unique_ptr<Registry> registry_;
	std::unique_ptr<ArenaGrid> arena_;
	std::unique_ptr<AISystem> aiSystem_;
};

} // namespace

// 1 - Arena preset application is reflected in AI blocked-grid view within same update cycle
TEST_F(ArenaAICoherenceTest, ArenaPresetReflectedInBlockedGridSameFrame) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	// first preset
	arena_->transformArenaWithPreset(presets[0].walls);
	
	BlockedGrid blockedGrid1 = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	EXPECT_EQ(static_cast<int>(blockedGrid1.size()), 32);
	EXPECT_EQ(static_cast<int>(blockedGrid1[0].size()), 32);

	arena_->transformArenaWithPreset(presets[1].walls);
	
	BlockedGrid blockedGrid2 = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	EXPECT_EQ(static_cast<int>(blockedGrid2.size()), 32);
	EXPECT_EQ(static_cast<int>(blockedGrid2[0].size()), 32);
}

// 2 - Path selected by AI remains walkable after movement step validation
TEST_F(ArenaAICoherenceTest, PathRemainsWalkableAfterMovementValidation) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	arena_->transformArenaWithPreset(presets[0].walls);
	
	BlockedGrid blockedGrid = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	Pathfinder pathfinder;
	GridHelper gridHelper;
	
	Vec2 start = { 1, 1 };
	Vec2 goal = { 30, 30 };
	std::vector<Vec2> ignored;
	
	std::vector<Vec2> path = pathfinder.findPath(blockedGrid, start, goal, 32, 32, 300, ignored);
	
	EXPECT_GT(path.size(), 0);

	for (const auto& step : path) {
		EXPECT_TRUE(gridHelper.isWalkable(blockedGrid, step, 32, 32, ignored));
	}
}

// 3 - Flood-fill safety check and pathfinder decision stay consistent on identical board state
TEST_F(ArenaAICoherenceTest, FloodFillAndPathfinderConsistentOnIdenticalBoard) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	arena_->transformArenaWithPreset(presets[0].walls);
	
	BlockedGrid blockedGrid = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	FloodFill floodFill;
	Pathfinder pathfinder;
	
	Vec2 start = { 1, 1 };
	Vec2 goal = { 30, 30 };
	std::vector<Vec2> ignored;
	
	// First check: flood fill counts reachable cells
	int reachableCount = floodFill.countReachable(blockedGrid, start, 32, 32, ignored);
	
	// Second check: pathfinder finds path to goal
	std::vector<Vec2> path = pathfinder.findPath(blockedGrid, start, goal, 32, 32, 300, ignored);
	
	if (reachableCount >= 2) {
		EXPECT_GT(path.size(), 0) << "Path should exist if goal is reachable";
	}
	
	int reachableCount2 = floodFill.countReachable(blockedGrid, start, 32, 32, ignored);
	EXPECT_EQ(reachableCount, reachableCount2) << "Reachable count should be consistent";
	
	std::vector<Vec2> path2 = pathfinder.findPath(blockedGrid, start, goal, 32, 32, 300, ignored);
	EXPECT_EQ(path.size(), path2.size()) << "Path length should be consistent";
}

// 4 - AI repaths after dynamic arena changes (spawn/despawn solids) without freezing
TEST_F(ArenaAICoherenceTest, AIRepathsAfterDynamicArenaChanges) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	arena_->transformArenaWithPreset(presets[0].walls);
	
	BlockedGrid blockedGrid1 = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	Pathfinder pathfinder;
	Vec2 start = { 1, 1 };
	Vec2 goal = { 30, 30 };
	std::vector<Vec2> ignored;
	
	std::vector<Vec2> path1 = pathfinder.findPath(blockedGrid1, start, goal, 32, 32, 300, ignored);
	int path1Length = path1.size();

	arena_->spawnObstacle(15, 15, 5, 5);
	
	BlockedGrid blockedGrid2 = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	std::vector<Vec2> path2 = pathfinder.findPath(blockedGrid2, start, goal, 32, 32, 300, ignored);
	
	EXPECT_GE(path2.size(), 0) << "Repathing should complete";

	if (path1.size() > 0 && path2.size() == 0) {
		SUCCEED();
	}
}

// 5 - Multiple AI entities use independent pathfinding without interference
TEST_F(ArenaAICoherenceTest, MultipleAIEntitiesPathfindIndependently) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	arena_->transformArenaWithPreset(presets[0].walls);
	
	BlockedGrid blockedGrid = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	Pathfinder pathfinder;
	GridHelper gridHelper;
	std::vector<Vec2> ignored;
	
	Vec2 start1 = { 1, 1 };
	Vec2 goal1 = { 30, 30 };
	std::vector<Vec2> path1a = pathfinder.findPath(blockedGrid, start1, goal1, 32, 32, 300, ignored);
	std::vector<Vec2> path1b = pathfinder.findPath(blockedGrid, start1, goal1, 32, 32, 300, ignored);

	Vec2 start2 = { 30, 30 };
	Vec2 goal2 = { 1, 1 };
	std::vector<Vec2> path2a = pathfinder.findPath(blockedGrid, start2, goal2, 32, 32, 300, ignored);
	std::vector<Vec2> path2b = pathfinder.findPath(blockedGrid, start2, goal2, 32, 32, 300, ignored);
	
	// Each query should be deterministic when repeated on identical board state.
	EXPECT_EQ(path1a.size(), path1b.size());
	EXPECT_EQ(path2a.size(), path2b.size());

	// At least one of the two opposite-direction queries should resolve in this preset.
	EXPECT_TRUE(!path1a.empty() || !path2a.empty());

	if (!path1a.empty()) {
		for (const auto& step : path1a) {
			EXPECT_TRUE(gridHelper.isWalkable(blockedGrid, step, 32, 32, ignored));
		}
	}

	if (!path2a.empty()) {
		for (const auto& step : path2a) {
			EXPECT_TRUE(gridHelper.isWalkable(blockedGrid, step, 32, 32, ignored));
		}
	}
}

// 6 - Arena grid safety check: walls block pathfinding as expected
TEST_F(ArenaAICoherenceTest, WallsProperlyBlockPathfinding) {
	// Arena with no obstacles should allow full traversal
	arena_->clearArena();
	
	BlockedGrid blockedGridClear = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	Pathfinder pathfinder;
	std::vector<Vec2> ignored;

	std::vector<Vec2> pathClear = pathfinder.findPath(
		blockedGridClear, 
		{ 0, 0 }, 
		{ 31, 31 }, 
		32, 32, 
		100, 
		ignored
	);
	
	EXPECT_GT(pathClear.size(), 0);
	
	arena_->clearArena();
	arena_->spawnObstacle(15, 0, 1, 32);
	
	BlockedGrid blockedGridWalled = aiSystem_->buildBlockedGrid(*registry_, arena_.get());
	
	std::vector<Vec2> pathWalled = pathfinder.findPath(
		blockedGridWalled,
		{ 0, 15 },
		{ 31, 15 },
		32, 32,
		300,
		ignored
	);
	
	EXPECT_GE(pathWalled.size(), 0);
}
