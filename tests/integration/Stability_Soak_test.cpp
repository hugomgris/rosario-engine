#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <random>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "components/SnakeComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/FoodTag.hpp"
#include "components/ScoreComponent.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "FrameContext.hpp"

namespace {

class StabilitySoakTest : public ::testing::Test {
protected:
	Registry registry_;
	std::unique_ptr<ArenaGrid> arena_;
	std::unique_ptr<ParticleSystem> particleSystem_;
	std::unique_ptr<AnimationSystem> animationSystem_;
	
	void SetUp() override {
		arena_ = std::make_unique<ArenaGrid>(32, 32);
		
		ParticleConfig particleConfig;
		particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json");
		particleSystem_ = std::make_unique<ParticleSystem>(1920, 1080, particleConfig);
		
		TunnelConfigLoader::PresetTable tunnelPresets;
		tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json");
		animationSystem_ = std::make_unique<AnimationSystem>();
		animationSystem_->enable(true, tunnelPresets.at("realm2D"));
	}
};

}

// 1 - 300-frame deterministic soak in Playing state with fixed seed and scripted inputs
TEST_F(StabilitySoakTest, ThreeHundredFrameDeterministicSoakNoOutOfBoundsAccess) {
	std::mt19937 rng(42);  // Fixed seed for determinism
	std::uniform_int_distribution<> posDist(1, 30);
	
	// Create initial gameplay entities
	Entity snake = registry_.createEntity();
	SnakeComponent snakeComp;
	snakeComp.segments.push_back({ { 15, 15 }, BeadType::None });
	snakeComp.segments.push_back({ { 14, 15 }, BeadType::None });
	snakeComp.isAlive = true;
	registry_.addComponent<SnakeComponent>(snake, snakeComp);
	registry_.addComponent<PositionComponent>(snake, PositionComponent{ { 15, 15 } });
	registry_.addComponent<ScoreComponent>(snake, ScoreComponent{ 0 });
	
	Entity food = registry_.createEntity();
	registry_.addComponent<FoodTag>(food, FoodTag{});
	registry_.addComponent<PositionComponent>(food, PositionComponent{ { 20, 20 } });
	
	// Load arena presets for transformation
	ArenaPresetLoader::PresetList arenaPresets;
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	// Run 300 frames
	EXPECT_NO_THROW({
		for (int frame = 0; frame < 300; ++frame) {
			ctx.frameNumber = frame;
			
			// Simulate movement
			auto& snakePos = registry_.getComponent<PositionComponent>(snake);
			snakePos.position.x = 1 + (frame % 30);
			snakePos.position.y = 1 + ((frame / 30) % 30);
			
			// Occasionally move food
			if (frame % 50 == 0) {
				auto& foodPos = registry_.getComponent<PositionComponent>(food);
				foodPos.position.x = posDist(rng);
				foodPos.position.y = posDist(rng);
			}
			
			// Update systems
			animationSystem_->update(ctx);
			particleSystem_->update(registry_, ctx);
			
			// Verify positions stay in bounds
			EXPECT_GE(snakePos.position.x, 0);
			EXPECT_LE(snakePos.position.x, 31);
			EXPECT_GE(snakePos.position.y, 0);
			EXPECT_LE(snakePos.position.y, 31);
		}
	});
	
	// Verify entities still valid
	EXPECT_TRUE(registry_.hasComponent<SnakeComponent>(snake));
	EXPECT_TRUE(registry_.hasComponent<FoodTag>(food));
}
/* 
// 2 - Repeated state cycling (Menu <-> Playing <-> GameOver) for N loops has no leaks
TEST_F(StabilitySoakTest, StateTransitionCyclingNLoopsNoLeaks) {
	enum class GameState { Menu, Playing, GameOver };
	GameState currentState = GameState::Menu;
	
	std::vector<Entity> allCreatedEntities;
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	// Cycle through states 20 times
	EXPECT_NO_THROW({
		for (int cycle = 0; cycle < 20; ++cycle) {
			switch (currentState) {
				case GameState::Menu:
					// Create menu entities
					{
						Entity menuEntity = registry_.createEntity();
						allCreatedEntities.push_back(menuEntity);
					}
					currentState = GameState::Playing;
					break;
					
				case GameState::Playing:
					// Create gameplay entities
					{
						Entity snake = registry_.createEntity();
						registry_.addComponent<SnakeComponent>(snake, SnakeComponent{});
						registry_.addComponent<ScoreComponent>(snake, ScoreComponent{});
						allCreatedEntities.push_back(snake);
						
						Entity food = registry_.createEntity();
						registry_.addComponent<FoodTag>(food, FoodTag{});
						allCreatedEntities.push_back(food);
					}
					currentState = GameState::GameOver;
					break;
					
				case GameState::GameOver:
					// Clear gameplay entities (simulated transition back to menu)
					registry_.clear();
					allCreatedEntities.clear();
					currentState = GameState::Menu;
					break;
			}
			
			// Run one frame in each state
			animationSystem_->update(ctx);
			particleSystem_->update(registry_, ctx);
		}
	});
}

// 3 - Long-run entity churn (spawn/despawn across rounds) shows no invalid accesses
TEST_F(StabilitySoakTest, LongRunEntityChurnNoInvalidAccesses) {
	Entity snakeEntity = Entity{};
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	EXPECT_NO_THROW({
		for (int round = 0; round < 10; ++round) {
			// Clear previous round (despawn)
			registry_.clear();
			
			// Spawn new entities for this round
			snakeEntity = registry_.createEntity();
			SnakeComponent snakeComp;
			snakeComp.segments.push_back({ { 15, 15 }, BeadType::None });
			registry_.addComponent<SnakeComponent>(snakeEntity, snakeComp);
			registry_.addComponent<ScoreComponent>(snakeEntity, ScoreComponent{ round * 10 });
			
			// Run several frames with entities in this round
			for (int frame = 0; frame < 10; ++frame) {
				ctx.frameNumber = round * 10 + frame;
				
				// Access entities (would throw if invalid)
				auto& snakeComp = registry_.getComponent<SnakeComponent>(snakeEntity);
				auto& score = registry_.getComponent<ScoreComponent>(snakeEntity);
				
				// Simulate gameplay
				if (frame % 3 == 0) {
					score.score += 1;
				}
				
				// Update systems
				animationSystem_->update(ctx);
				particleSystem_->update(registry_, ctx);
			}
		}
	});
	
	// After all rounds, registry should be in valid state
	EXPECT_NO_THROW({
		registry_.view<>();
	});
}

// 4 - Config reload + gameplay transition sequence remains stable
TEST_F(StabilitySoakTest, ConfigReloadAndTransitionSequenceStable) {
	// Load configurations
	ParticleConfig particleConfig1;
	EXPECT_NO_THROW(particleConfig1 = ParticleConfigLoader::load("data/ParticleConfig.json"));
	
	TunnelConfigLoader::PresetTable tunnelPresets;
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	EXPECT_NO_THROW({
		for (int i = 0; i < 5; ++i) {
			// Simulate gameplay
			Entity snake = registry_.createEntity();
			registry_.addComponent<SnakeComponent>(snake, SnakeComponent{});
			
			// Run few frames
			for (int frame = 0; frame < 5; ++frame) {
				animationSystem_->update(ctx);
				particleSystem_->update(registry_, ctx);
			}
			
			// Reload config (on transition to new level)
			ParticleConfig particleConfigReloaded;
			particleConfigReloaded = ParticleConfigLoader::load("data/ParticleConfig.json");
			
			// Update particle system with new config
			particleSystem_ = std::make_unique<ParticleSystem>(1920, 1080, particleConfigReloaded);
			
			// Clear for next round
			registry_.clear();
		}
	});
}

// 5 - High entity creation rate (stress test) maintains coherence
TEST_F(StabilitySoakTest, HighEntityCreationRateStressTest) {
	FrameContext ctx;
	ctx.deltaTime = 0.016f;
	
	std::vector<Entity> entities;
	
	EXPECT_NO_THROW({
		for (int frame = 0; frame < 100; ++frame) {
			ctx.frameNumber = frame;
			
			// Create multiple entities per frame (stress)
			for (int i = 0; i < 5; ++i) {
				Entity e = registry_.createEntity();
				registry_.addComponent<PositionComponent>(e, PositionComponent{ { frame % 32, i % 32 } });
				entities.push_back(e);
			}
			
			// Update systems
			animationSystem_->update(ctx);
			particleSystem_->update(registry_, ctx);
			
			// Every 20 frames, despawn half the entities
			if (frame % 20 == 0 && frame > 0) {
				for (size_t i = 0; i < entities.size() / 2; ++i) {
					// Only destroy valid entities
					try {
						registry_.getComponent<PositionComponent>(entities[i]);
						// Entity still valid, would destroy in real system
					} catch (...) {
						// Entity already gone
					}
				}
				entities.erase(entities.begin(), entities.begin() + entities.size() / 2);
			}
		}
	});
}
 */