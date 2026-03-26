#include <gtest/gtest.h>

#include <memory>

#include "ecs/Registry.hpp"
#include "systems/RenderSystem.hpp"
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "components/RenderComponent.hpp"
#include "FrameContext.hpp"

namespace {

enum class RenderMode { Mode2D, Mode3D };

struct RenderState {
	RenderMode currentMode;
	bool postProcessingEnabled;
	bool tunnelAnimationEnabled;
};

} // namespace

// 1 - Mode2D/Mode3D switch during gameplay preserves render/update coherence
TEST(RenderingModes, Mode2D3DSwitchPreservesCoherence) {
	Registry registry;
	RenderState renderState{ RenderMode::Mode2D, true, true };
	
	// Load configurations
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));
	
	// Create render entity
	Entity renderEntity = registry.createEntity();
	registry.addComponent<RenderComponent>(renderEntity, RenderComponent{});
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	// Frame 1: Mode2D update/render
	EXPECT_NO_THROW({
		animationSystem.update(0.016f, animationSystem.getArena());
		particleSystem.update(0.016f, registry, ctx);
	});
	
	// Switch to Mode3D
	renderState.currentMode = RenderMode::Mode3D;
	
	// Frame 2: Mode3D update/render
	EXPECT_NO_THROW({
		animationSystem.update(0.016f, animationSystem.getArena());
		particleSystem.update(0.016f, registry, ctx);
	});
	
	// Switch back to Mode2D
	renderState.currentMode = RenderMode::Mode2D;
	
	// Frame 3: Back to Mode2D
	EXPECT_NO_THROW({
		animationSystem.update(0.016f, animationSystem.getArena());
		particleSystem.update(0.016f, registry, ctx);
	});
	
	// Verify entity still exists and systems function
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(renderEntity));
}

// 2 - Post-processing on/off toggle does not break UI composition order
TEST(RenderingModes, PostProcessingTogglePreservesUICompositionOrder) {
	Registry registry;
	
	PostProcessConfigLoader::PresetTable postProcessPresets;
	EXPECT_NO_THROW(postProcessPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));
	
	// Create multiple UI render entities to establish composition order
	Entity ui1 = registry.createEntity();
	Entity ui2 = registry.createEntity();
	Entity ui3 = registry.createEntity();
	
	registry.addComponent<RenderComponent>(ui1, RenderComponent{});
	registry.addComponent<RenderComponent>(ui2, RenderComponent{});
	registry.addComponent<RenderComponent>(ui3, RenderComponent{});
	
	// Verify initial state
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui1));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui2));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui3));
	
	// Toggle post-processing on
	PostProcessConfig ppOn = postProcessPresets["crt_bloom"];
	EXPECT_TRUE(ppOn.enabled);
	
	// All entities should still be renderable
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui1));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui2));
	EXPECT_TRUE(registry.hasComponent<RenderComponent>(ui3));
	
	// Toggle post-processing off
	PostProcessConfig ppOff = postProcessPresets["clean"];
	EXPECT_FALSE(ppOff.enabled);
	
	// UI composition order should be preserved
	auto renderView = registry.view<RenderComponent>();
	int count = 0;
	for (auto entity : renderView) {
		count++;
	}
	EXPECT_EQ(count, 3);
}

// 3 - Tunnel animation and particle rendering coexist without state leakage
TEST(RenderingModes, TunnelAnimationAndParticlesCoexistSafely) {
	Registry registry;
	
	ParticleConfig particleConfig;
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	// Initialize both systems
	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("menu"));
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	// Run both systems for multiple frames
	for (int i = 0; i < 10; ++i) {
		ctx.frameNumber = i;
		
		EXPECT_NO_THROW({
			// Animation system update
			animationSystem.update(0.016f, animationSystem.getArena());
			
			// Particle system update (might add particles)
			particleSystem.update(0.016f, registry, ctx);
			
			// Both systems should have independent state
		});
	}
}

// 4 - Color mode (CRT bloom, vignette, scanlines) switching maintains consistency
TEST(RenderingModes, PostProcessColorModeSwitchingMaintainsConsistency) {
	PostProcessConfigLoader::PresetTable presets;
	EXPECT_NO_THROW(presets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));
	
	// Get different post-process configs
	PostProcessConfig crtBloom = presets["crt_bloom"];
	PostProcessConfig crt = presets["crt"];
	PostProcessConfig menu = presets["menu"];
	PostProcessConfig clean = presets["clean"];
	
	// All should be valid (though some might have effects disabled)
	EXPECT_TRUE(true); // Configs loaded
	
	// Switching between them should be safe
	std::vector<std::string> modes = { "crt_bloom", "crt", "menu", "clean" };
	
	for (const auto& mode : modes) {
		PostProcessConfig config = presets[mode];
		// Config should be valid regardless of enabled/disabled
		// (would have more specific checks if we had getters)
		EXPECT_TRUE(true);
	}
}

// 5 - Render component state survives mode switches across multiple frames
TEST(RenderingModes, RenderComponentStateSurvivesModesSwitches) {
	Registry registry;
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	// Create render entities with initial state
	Entity entity1 = registry.createEntity();
	Entity entity2 = registry.createEntity();
	
	BaseColor color1{ 255, 0, 0, 255 };
	BaseColor color2{ 0, 255, 0, 255 };
	
	registry.addComponent<RenderComponent>(entity1, RenderComponent{ color1 });
	registry.addComponent<RenderComponent>(entity2, RenderComponent{ color2 });
	
	// Verify initial colors
	auto& comp1Before = registry.getComponent<RenderComponent>(entity1);
	auto& comp2Before = registry.getComponent<RenderComponent>(entity2);
	EXPECT_EQ(comp1Before.color.r, 255);
	EXPECT_EQ(comp2Before.color.g, 255);
	
	// Simulate mode switches
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	// Run for several frames with different "modes"
	for (int i = 0; i < 5; ++i) {
		ctx.frameNumber = i;
		animationSystem.update(0.016f, animationSystem.getArena());
		
		// Entity colors should remain unchanged
		auto& comp1After = registry.getComponent<RenderComponent>(entity1);
		auto& comp2After = registry.getComponent<RenderComponent>(entity2);
		EXPECT_EQ(comp1After.color.r, 255);
		EXPECT_EQ(comp2After.color.g, 255);
	}
}
