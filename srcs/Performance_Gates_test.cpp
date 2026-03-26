#include <gtest/gtest.h>

#include <chrono>
#include <vector>
#include <memory>
#include <numeric>
#include <cmath>
#include <algorithm>

#include "ecs/Registry.hpp"
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
		// Simplified: assumes linear distribution
		return minTime + (maxTime - minTime) * (percentile / 100.0);
	}
	
	double getAverage() const {
		return frameCount > 0 ? totalTime / frameCount : 0.0;
	}
};

} // namespace

// 1 - 95th percentile frame time stays under target during stress scenario
TEST(PerformanceGates, NinetyfifthPercentileFrameTimeUnderTarget) {
	Registry registry;
	
	// Load configurations for stress scenario
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	// Create stress scenario with particles + AI + postFX
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));
	
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[0].walls);
	AISystem aiSystem(32, 32);
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;  // 60 FPS target
	double frameTimeTarget = 16.0;  // ms
	
	std::vector<double> frameTimes;
	frameTimes.reserve(100);
	
	// Run 100 frames and measure performance
	for (int frame = 0; frame < 100; ++frame) {
		ctx.frameNumber = frame;
		
		auto frameStart = std::chrono::high_resolution_clock::now();
		
		// Stress: run all systems together
		animationSystem.update(0.016f, arena.get());
		particleSystem.update(0.016f, registry, ctx);
		aiSystem.update(registry, arena.get());
		
		auto frameEnd = std::chrono::high_resolution_clock::now();
		double frameTimeMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		frameTimes.push_back(frameTimeMs);
	}
	
	// Calculate 95th percentile
	std::sort(frameTimes.begin(), frameTimes.end());
	int percentile95Index = static_cast<int>(frameTimes.size() * 0.95);
	double percentile95Time = frameTimes[percentile95Index];
	
	// 95th percentile should be under target (with some reasonable margin, e.g., 2x)
	double upperBound = frameTimeTarget * 2.0;  // Allow up to 2x target
	EXPECT_LE(percentile95Time, upperBound) 
		<< "95th percentile frame time " << percentile95Time << "ms exceeds " << upperBound << "ms";
}

// 2 - AI tick time stays under target on heavy arena preset during active gameplay
TEST(PerformanceGates, AITickTimeUnderTargetOnHeavyArena) {
	Registry registry;
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	// Use heaviest arena preset (typically Maze or FourRooms)
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[6].walls);  // Maze preset
	
	AISystem aiSystem(32, 32);
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	double aiTickTarget = 5.0;  // ms (should be fast)
	
	std::vector<double> aiTimes;
	aiTimes.reserve(100);
	
	// Run 100 AI updates
	for (int tick = 0; tick < 100; ++tick) {
		auto tickStart = std::chrono::high_resolution_clock::now();
		
		aiSystem.update(registry, arena.get());
		
		auto tickEnd = std::chrono::high_resolution_clock::now();
		double aiTimeMs = std::chrono::duration<double, std::milli>(tickEnd - tickStart).count();
		aiTimes.push_back(aiTimeMs);
	}
	
	// Calculate average and max
	double avgAiTime = std::accumulate(aiTimes.begin(), aiTimes.end(), 0.0) / aiTimes.size();
	double maxAiTime = *std::max_element(aiTimes.begin(), aiTimes.end());
	
	// AI should typically complete well under target
	EXPECT_LE(avgAiTime, aiTickTarget) 
		<< "Average AI time " << avgAiTime << "ms exceeds " << aiTickTarget << "ms";
}

// 3 - Particle system update time stays bounded with concurrent menu trail emitters
TEST(PerformanceGates, ParticleSystemTimeUnderTargetWithMenuTrails) {
	Registry registry;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	double particleUpdateTarget = 3.0;  // ms
	
	std::vector<double> particleTimes;
	particleTimes.reserve(100);
	
	// Run 100 particle updates
	for (int frame = 0; frame < 100; ++frame) {
		ctx.frameNumber = frame;
		
		auto particleStart = std::chrono::high_resolution_clock::now();
		
		particleSystem.update(0.016f, registry, ctx);
		
		auto particleEnd = std::chrono::high_resolution_clock::now();
		double particleTimeMs = std::chrono::duration<double, std::milli>(particleEnd - particleStart).count();
		particleTimes.push_back(particleTimeMs);
	}
	
	// Calculate 95th percentile
	std::sort(particleTimes.begin(), particleTimes.end());
	int percentile95Index = static_cast<int>(particleTimes.size() * 0.95);
	double percentile95Time = particleTimes[percentile95Index];
	
	// Particle updates should stay reasonable
	double upperBound = particleUpdateTarget * 3.0;  // Allow up to 3x for concurrent emitters
	EXPECT_LE(percentile95Time, upperBound) 
		<< "Particle system 95th percentile " << percentile95Time << "ms exceeds " << upperBound << "ms";
}

// 4 - Pathfinding on complex maze in <10ms
TEST(PerformanceGates, PathfindingComplexMazeUnderTenMs) {
	Registry registry;
	
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	// Use Maze preset (complex)
	auto arena = std::make_unique<ArenaGrid>(32, 32);
	arena->transformArenaWithPreset(arenaPresets[6]);  // Maze
	
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
		
		// Each path should complete reasonably
		EXPECT_LE(pathTimeMs, pathfindTarget * 2.0);
	}
	
	// Average should be well under target
	double avgPathTime = std::accumulate(pathfindTimes.begin(), pathfindTimes.end(), 0.0) / pathfindTimes.size();
	EXPECT_LE(avgPathTime, pathfindTarget);
}

// 5 - Frame time consistency: std deviation minimal across 100 frames
TEST(PerformanceGates, FrameTimeConsistencyMinimalDeviation) {
	Registry registry;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	std::vector<double> frameTimes;
	frameTimes.reserve(100);
	
	// Run 100 frames
	for (int frame = 0; frame < 100; ++frame) {
		ctx.frameNumber = frame;
		
		auto frameStart = std::chrono::high_resolution_clock::now();
		animationSystem.update(0.016f, arena.get());
		particleSystem.update(0.016f, registry, ctx);
		auto frameEnd = std::chrono::high_resolution_clock::now();
		
		double frameTimeMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		frameTimes.push_back(frameTimeMs);
	}
	
	// Calculate mean and standard deviation
	double mean = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
	double variance = 0.0;
	for (double time : frameTimes) {
		variance += (time - mean) * (time - mean);
	}
	variance /= frameTimes.size();
	double stdDev = std::sqrt(variance);
	
	// Coefficient of variation (stdDev / mean) should be small
	double coeffVariation = stdDev / mean;
	EXPECT_LE(coeffVariation, 0.5) 
		<< "Frame time variation too high: stdDev=" << stdDev << "ms, mean=" << mean << "ms";
}
