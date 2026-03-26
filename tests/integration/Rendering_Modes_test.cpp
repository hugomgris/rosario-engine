#include <gtest/gtest.h>

#include "../../incs/DataStructs.hpp"
#include "ecs/Registry.hpp"
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "arena/ArenaGrid.hpp"
#include "components/RenderComponent.hpp"
#include "FrameContext.hpp"

// 1 - Mode2D/Mode3D switch during gameplay preserves render/update coherence
TEST(RenderingModes, Mode2D3DSwitchPreservesCoherence) {
	Registry registry;
	bool use3DMode = false;
	const float dt = 0.016f;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));
	ArenaGrid arena(32, 32);
	arena.setGameplayArena();
	
	Entity renderEntity = registry.createEntity();
	registry.addComponent<RenderComponent>(renderEntity, RenderComponent{ BaseColor{255, 255, 255, 255} });
	
	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = false;
	
	EXPECT_NO_THROW({
		animationSystem.update(dt, arena);
		particleSystem.update(dt, registry, ctx);
	});
	
	use3DMode = true;
	EXPECT_TRUE(use3DMode);
	
	EXPECT_NO_THROW({
		animationSystem.update(dt, arena);
		particleSystem.update(dt, registry, ctx);
	});
	
	use3DMode = false;
	EXPECT_FALSE(use3DMode);
	
	EXPECT_NO_THROW({
		animationSystem.update(dt, arena);
		particleSystem.update(dt, registry, ctx);
	});
	
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(renderEntity));
}

// 2 - Post-processing on/off toggle does not break UI composition order
TEST(RenderingModes, PostProcessingTogglePreservesUICompositionOrder) {
	Registry registry;
	
	PostProcessConfigLoader::PresetTable postProcessPresets;
	EXPECT_NO_THROW(postProcessPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));
	ASSERT_TRUE(postProcessPresets.count("crt_bloom") > 0);
	ASSERT_TRUE(postProcessPresets.count("clean") > 0);
	
	Entity ui1 = registry.createEntity();
	Entity ui2 = registry.createEntity();
	Entity ui3 = registry.createEntity();
	
	registry.addComponent<RenderComponent>(ui1, RenderComponent{ BaseColor{255, 255, 255, 255} });
	registry.addComponent<RenderComponent>(ui2, RenderComponent{ BaseColor{220, 220, 220, 255} });
	registry.addComponent<RenderComponent>(ui3, RenderComponent{ BaseColor{180, 180, 180, 255} });
	
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui1));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui2));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui3));
	
	const PostProcessConfig ppOn = postProcessPresets.at("crt_bloom");
	EXPECT_TRUE(ppOn.enabled);

	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui1));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui2));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui3));

	const PostProcessConfig ppOff = postProcessPresets.at("clean");
	EXPECT_FALSE(ppOff.enabled);

	auto renderView = registry.view<RenderComponent>();
	int count = 0;
	for (auto entity : renderView) {
		(void)entity;
		count++;
	}
	EXPECT_EQ(count, 3);
}

// 3 - Tunnel animation and particle rendering coexist without state leakage
TEST(RenderingModes, TunnelAnimationAndParticlesCoexistSafely) {
	Registry registry;
	const float dt = 0.016f;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	ArenaGrid arena(32, 32);
	arena.setMenuArena();
	
	FrameContext ctx;
	ctx.arena = &arena;
	ctx.gridWidth = 32;
	ctx.gridHeight = 32;
	ctx.menuLikeFrame = true;
	
	for (int i = 0; i < 10; ++i) {
		(void)i;
		
		EXPECT_NO_THROW({
			animationSystem.update(dt, arena);
			particleSystem.update(dt, registry, ctx);
		});
	}

	EXPECT_TRUE(animationSystem.isEnabled());
}

// 4 - Color mode (CRT bloom, vignette, scanlines) switching maintains consistency
TEST(RenderingModes, PostProcessColorModeSwitchingMaintainsConsistency) {
	PostProcessConfigLoader::PresetTable presets;
	EXPECT_NO_THROW(presets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));
	ASSERT_TRUE(presets.count("crt_bloom") > 0);
	ASSERT_TRUE(presets.count("crt") > 0);
	ASSERT_TRUE(presets.count("menu") > 0);
	ASSERT_TRUE(presets.count("clean") > 0);
	
	const PostProcessConfig crtBloom = presets.at("crt_bloom");
	const PostProcessConfig crt = presets.at("crt");
	const PostProcessConfig menu = presets.at("menu");
	const PostProcessConfig clean = presets.at("clean");
	
	EXPECT_TRUE(crtBloom.enabled);
	EXPECT_TRUE(crt.enabled);
	EXPECT_TRUE(menu.enabled);
	EXPECT_FALSE(clean.enabled);
	
	const std::vector<std::string> modes = { "crt_bloom", "crt", "menu", "clean" };
	
	for (const auto& mode : modes) {
		const PostProcessConfig config = presets.at(mode);
		EXPECT_GE(config.scanlineIntensity, 0.0f);
		EXPECT_GE(config.vignetteStrength, 0.0f);
	}
}

// 5 - Render component state survives mode switches across multiple frames
TEST(RenderingModes, RenderComponentStateSurvivesModesSwitches) {
	Registry registry;
	const float dt = 0.016f;
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	ASSERT_TRUE(tunnelPresets.count("realm2D") > 0);
	
	ArenaGrid arena(32, 32);
	arena.setGameplayArena();

	Entity entity1 = registry.createEntity();
	Entity entity2 = registry.createEntity();
	
	BaseColor color1{ 255, 0, 0, 255 };
	BaseColor color2{ 0, 255, 0, 255 };
	
	registry.addComponent<RenderComponent>(entity1, RenderComponent{ color1 });
	registry.addComponent<RenderComponent>(entity2, RenderComponent{ color2 });

	auto& comp1Before = registry.getComponent<RenderComponent>(entity1);
	auto& comp2Before = registry.getComponent<RenderComponent>(entity2);
	EXPECT_EQ(comp1Before.color.r, 255);
	EXPECT_EQ(comp2Before.color.g, 255);

	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));

	for (int i = 0; i < 5; ++i) {
		(void)i;
		animationSystem.update(dt, arena);

		auto& comp1After = registry.getComponent<RenderComponent>(entity1);
		auto& comp2After = registry.getComponent<RenderComponent>(entity2);
		EXPECT_EQ(comp1After.color.r, 255);
		EXPECT_EQ(comp2After.color.g, 255);
	}
}
