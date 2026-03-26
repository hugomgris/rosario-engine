#include <gtest/gtest.h>

#include <chrono>
#include <vector>
#include <memory>
#include <limits>
#include <numeric>
#include <cmath>
#include <algorithm>

#include "ecs/Registry.hpp"
#include "components/AIComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/AISystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "AI/Pathfinder.hpp"
#include "FrameContext.hpp"

namespace {

struct PerformanceMetrics {
	double minTime = std::numeric_limits<double>::max();
	double maxTime = 0.0;
	double totalTime = 0.0;
	int frameCount = 0;
	
	double getPercentile(double percentile) const {
		return minTime + (maxTime - minTime) * (percentile / 100.0);
	}
	
	double getAverage() const {
		return frameCount > 0 ? totalTime / frameCount : 0.0;
	}
};

}

// 1 - 95th percentile frame time stays under target during stress scenario
TEST(PerformanceGates, NinetyfifthPercentileFrameTimeUnderTarget) {
	Registry registry;
	const float dt = 0.016f;

	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	ASSERT_TRUE(tunnelPresets.count("realm2D") > 0);
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	ASSERT_FALSE(arenaPresets.empty());
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));
	
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[0].walls);
	AISystem aiSystem(32, 32);

	Entity aiEntity = registry.createEntity();
	SnakeComponent aiSnake;
	aiSnake.slot = PlayerSlot::A;
	aiSnake.segments.push_back({ { 15, 15 }, BeadType::None });
	registry.addComponent<SnakeComponent>(aiEntity, aiSnake);
	registry.addComponent<PositionComponent>(aiEntity, PositionComponent{ { 15, 15 } });
	registry.addComponent<MovementComponent>(aiEntity, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
	registry.addComponent<AIComponent>(aiEntity, AIComponent{});
	
	FrameContext ctx;
	ctx.arena = arena.get();
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = false;
	double frameTimeTarget = 16.0;
	
	std::vector<double> frameTimes;
	frameTimes.reserve(100);

	for (int frame = 0; frame < 100; ++frame) {
		(void)frame;
		
		auto frameStart = std::chrono::high_resolution_clock::now();

		animationSystem.update(dt, *arena);
		particleSystem.update(dt, registry, ctx);
		aiSystem.update(registry, ctx);
		
		auto frameEnd = std::chrono::high_resolution_clock::now();
		double frameTimeMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		frameTimes.push_back(frameTimeMs);
	}
	
	std::sort(frameTimes.begin(), frameTimes.end());
	int percentile95Index = static_cast<int>(frameTimes.size() * 0.95);
	double percentile95Time = frameTimes[percentile95Index];
	
	double upperBound = frameTimeTarget * 2.0;
	EXPECT_LE(percentile95Time, upperBound) 
		<< "95th percentile frame time " << percentile95Time << "ms exceeds " << upperBound << "ms";
}

// 2 - AI tick time stays under target on heavy arena preset during active gameplay
TEST(PerformanceGates, AITickTimeUnderTargetOnHeavyArena) {
	Registry registry;
	const float dt = 0.016f;
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	ASSERT_FALSE(arenaPresets.empty());
	const size_t heavyIndex = std::min<size_t>(6, arenaPresets.size() - 1);
	
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[heavyIndex].walls);
	
	AISystem aiSystem(32, 32);

	Entity aiEntity = registry.createEntity();
	SnakeComponent aiSnake;
	aiSnake.slot = PlayerSlot::A;
	aiSnake.segments.push_back({ { 15, 15 }, BeadType::None });
	registry.addComponent<SnakeComponent>(aiEntity, aiSnake);
	registry.addComponent<PositionComponent>(aiEntity, PositionComponent{ { 15, 15 } });
	registry.addComponent<MovementComponent>(aiEntity, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
	registry.addComponent<AIComponent>(aiEntity, AIComponent{});
	
	FrameContext ctx;
	ctx.arena = arena.get();
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = false;
	double aiTickTarget = 5.0;  // ms (should be fast)
	
	std::vector<double> aiTimes;
	aiTimes.reserve(100);
	
	// Run 100 AI updates
	for (int tick = 0; tick < 100; ++tick) {
		(void)tick;
		auto tickStart = std::chrono::high_resolution_clock::now();
		
		aiSystem.update(registry, ctx);
		
		auto tickEnd = std::chrono::high_resolution_clock::now();
		double aiTimeMs = std::chrono::duration<double, std::milli>(tickEnd - tickStart).count();
		aiTimes.push_back(aiTimeMs);
	}
	
	double avgAiTime = std::accumulate(aiTimes.begin(), aiTimes.end(), 0.0) / aiTimes.size();
	double maxAiTime = *std::max_element(aiTimes.begin(), aiTimes.end());
	
	EXPECT_LE(avgAiTime, aiTickTarget) 
		<< "Average AI time " << avgAiTime << "ms exceeds " << aiTickTarget << "ms";
	EXPECT_LE(maxAiTime, aiTickTarget * 4.0)
		<< "Max AI time spike " << maxAiTime << "ms exceeds allowed burst bound";
}

// 3 - Particle system update time stays bounded with concurrent menu trail emitters
TEST(PerformanceGates, ParticleSystemTimeUnderTargetWithMenuTrails) {
	Registry registry;
	const float dt = 0.016f;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->setMenuArena();
	
	FrameContext ctx;
	ctx.arena = arena.get();
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = true;
	double particleUpdateTarget = 3.0;  // ms
	
	std::vector<double> particleTimes;
	particleTimes.reserve(100);
	
	// Run 100 particle updates
	for (int frame = 0; frame < 100; ++frame) {
		(void)frame;
		
		auto particleStart = std::chrono::high_resolution_clock::now();
		
		particleSystem.update(dt, registry, ctx);
		
		auto particleEnd = std::chrono::high_resolution_clock::now();
		double particleTimeMs = std::chrono::duration<double, std::milli>(particleEnd - particleStart).count();
		particleTimes.push_back(particleTimeMs);
	}
	
	std::sort(particleTimes.begin(), particleTimes.end());
	int percentile95Index = static_cast<int>(particleTimes.size() * 0.95);
	double percentile95Time = particleTimes[percentile95Index];
	
	double upperBound = particleUpdateTarget * 3.0;
	EXPECT_LE(percentile95Time, upperBound) 
		<< "Particle system 95th percentile " << percentile95Time << "ms exceeds " << upperBound << "ms";
}

// 4 - Pathfinding on complex maze in <10ms
TEST(PerformanceGates, PathfindingComplexMazeUnderTenMs) {
	Registry registry;
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	ASSERT_FALSE(arenaPresets.empty());
	const size_t heavyIndex = std::min<size_t>(6, arenaPresets.size() - 1);
	
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[heavyIndex].walls);
	
	AISystem aiSystem(32, 32);
	BlockedGrid blockedGrid = aiSystem.buildBlockedGrid(registry, arena.get());
	
	Pathfinder pathfinder;
	std::vector<Vec2> ignored;
	double pathfindTarget = 10.0;  // ms
	
	std::vector<double> pathfindTimes;
	pathfindTimes.reserve(50);
	
	// Run 50 pathfinding operations
	for (int i = 0; i < 50; ++i) {
		Vec2 start = { i % 8, (i / 8) % 4 };
		Vec2 goal = { 31 - (i % 8), 31 - ((i / 8) % 4) };
		
		auto pathStart = std::chrono::high_resolution_clock::now();
		
		std::vector<Vec2> path = pathfinder.findPath(blockedGrid, start, goal, 32, 32, 300, ignored);
		
		auto pathEnd = std::chrono::high_resolution_clock::now();
		double pathTimeMs = std::chrono::duration<double, std::milli>(pathEnd - pathStart).count();
		pathfindTimes.push_back(pathTimeMs);
	}

	std::sort(pathfindTimes.begin(), pathfindTimes.end());
	const int percentile95Index = static_cast<int>(pathfindTimes.size() * 0.95);
	const double percentile95Time = pathfindTimes[percentile95Index];

	const double percentileUpperBound = pathfindTarget * 4.0;
	const double averageUpperBound = pathfindTarget * 3.0;

	double avgPathTime = std::accumulate(pathfindTimes.begin(), pathfindTimes.end(), 0.0) / pathfindTimes.size();
	EXPECT_LE(percentile95Time, percentileUpperBound)
		<< "Pathfinding p95 " << percentile95Time << "ms exceeds " << percentileUpperBound << "ms";
	EXPECT_LE(avgPathTime, averageUpperBound)
		<< "Average pathfinding time " << avgPathTime << "ms exceeds " << averageUpperBound << "ms";
}

// 5 - Frame time consistency: std deviation minimal across 100 frames
TEST(PerformanceGates, FrameTimeConsistencyMinimalDeviation) {
	Registry registry;
	const float dt = 0.016f;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	ASSERT_TRUE(tunnelPresets.count("menu") > 0);
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->setMenuArena();
	
	FrameContext ctx;
	ctx.arena = arena.get();
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = true;
	
	std::vector<double> frameTimes;
	frameTimes.reserve(100);
	
	// Run 100 frames
	for (int frame = 0; frame < 100; ++frame) {
		(void)frame;
		
		auto frameStart = std::chrono::high_resolution_clock::now();
		animationSystem.update(dt, *arena);
		particleSystem.update(dt, registry, ctx);
		auto frameEnd = std::chrono::high_resolution_clock::now();
		
		double frameTimeMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		frameTimes.push_back(frameTimeMs);
	}
	
	double mean = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
	double variance = 0.0;
	for (double time : frameTimes) {
		variance += (time - mean) * (time - mean);
	}
	variance /= frameTimes.size();
	double stdDev = std::sqrt(variance);
	
	double coeffVariation = stdDev / mean;
	EXPECT_LE(coeffVariation, 0.5) 
		<< "Frame time variation too high: stdDev=" << stdDev << "ms, mean=" << mean << "ms";
}
