#include <gtest/gtest.h>

#include <memory>

#include "ecs/Registry.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/AISystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "AI/AIPresetLoader.hpp"
#include "FrameContext.hpp"

// 1 - One full frame in Menu state runs update+render with no crashes
TEST(FramePipeline, MenuStateFrameUpdateRenderNoCrash) {
	Registry registry;
	const float dt = 0.016f;

	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));

	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));

	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	ArenaGrid arena(60, 33);
	arena.setMenuArena();

	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 60;
	ctx.gridHeight = 33;
	ctx.menuLikeFrame = true;

	EXPECT_NO_THROW({
		animationSystem.update(dt, arena);
		particleSystem.update(dt, registry, ctx);
	});
}

// 2 - One full frame in Playing state runs update+render with no crashes
TEST(FramePipeline, PlayingStateFrameUpdateRenderNoCrash) {
	Registry registry;
	const float dt = 0.016f;

	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));

	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));

	AIPresetLoader::PresetTable aiPresets;
	EXPECT_NO_THROW(aiPresets = AIPresetLoader::load("data/AIPresets.json"));
	(void)aiPresets;

	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));

	CollisionRuleTable collisionRules;
	EXPECT_NO_THROW(collisionRules = CollisionRuleLoader::load("data/CollisionRules.json"));

	ArenaGrid arena(32, 32);
	arena.transformArenaWithPreset(arenaPresets[0].walls);

	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));

	AISystem aiSystem(32, 32);
	CollisionSystem collisionSystem;
	MovementSystem movementSystem;
	CollisionEffectDispatcher dispatcher;
	dispatcher.registerDefaults();

	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = false;
	GameState state = GameState::Playing;
	ctx.state = &state;

	EXPECT_NO_THROW({
		aiSystem.update(registry, ctx);
		movementSystem.update(registry, dt);
		collisionSystem.update(registry, collisionRules, dispatcher, ctx);
		particleSystem.update(dt, registry, ctx);
		animationSystem.update(dt, arena);
	});
}

// 3 - One full frame in GameOver state runs update+render with no crashes
TEST(FramePipeline, GameOverStateFrameUpdateRenderNoCrash) {
	Registry registry;
	const float dt = 0.016f;

	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));

	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));

	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	ArenaGrid arena(60, 33);
	arena.setMenuArena();

	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 60;
	ctx.gridHeight = 33;
	ctx.menuLikeFrame = true;

	EXPECT_NO_THROW({
		animationSystem.update(dt, arena);
		particleSystem.update(dt, registry, ctx);
	});
}

// 4 - Multiple consecutive frames maintain state consistency
TEST(FramePipeline, MultipleFramesStatConsistency) {
	Registry registry;
	const float dt = 0.016f;

	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));

	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));

	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	ArenaGrid arena(60, 33);
	arena.setMenuArena();

	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 60;
	ctx.gridHeight = 33;
	ctx.menuLikeFrame = true;

	// 10 consecutive frames
	EXPECT_NO_THROW({
		for (int i = 0; i < 10; ++i) {
			animationSystem.update(dt, arena);
			particleSystem.update(dt, registry, ctx);
		}
	});
}
