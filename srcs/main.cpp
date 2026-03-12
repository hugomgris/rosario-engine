#include <iostream>
#include <algorithm>
#include <ctime>
#include <raylib.h>

#include "../incs/DataStructs.hpp"
#include "../incs/Colors.hpp"
#include "../incs/RaylibColors.hpp"
#include "ecs/Registry.hpp"
#include "components/PositionComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "systems/InputSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/PostProcessingSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include "helpers/Factories.hpp"
#include "helpers/GameState.hpp"
#include "collision/CollisionRule.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "../incs/FrameContext.hpp"
#include "AI/AIPresetLoader.hpp"
#include "systems/AISystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "particles/ParticleConfigLoader.hpp"

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

int main() {
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	// Config json load
	CollisionRuleTable ruleTable = CollisionRuleLoader::load("data/collisionRules.json");
	AIPresetLoader::PresetTable AIPresets = AIPresetLoader::load("data/AIPresets.json");
	ParticleConfig particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json");

	// Dispatcher set up
	CollisionEffectDispatcher dispatcher;
	dispatcher.registerDefaults();

	// ECS core
	Registry registry;

	// Gameplay systems
	InputSystem				inputSystem;
	MovementSystem			movementSystem;
	CollisionSystem			collisionSystem;
	AISystem				aiSystem(GRID_W, GRID_H);
	RenderSystem            renderSystem;
	PostProcessingSystem    postProcessingSystem;
	ParticleSystem          particleSystem(SCREEN_W, SCREEN_H, particleConfig);

	renderSystem.init(GRID_W, GRID_H);
	postProcessingSystem.init(SCREEN_W, SCREEN_H);
	postProcessingSystem.setConfig(PostProcessingSystem::presetCRTBloom());

	// initial world
	ArenaGrid arena(GRID_W, GRID_H);
	Entity playerSnake(0u), secondSnake(0u), food(0u);
	GameState::resetGame(registry, inputSystem, playerSnake, secondSnake, food, GRID_W, GRID_H, arena, AIPresets, GameMode::VSAI);
	RenderMode renderMode = RenderMode::MODE2D;

	while (true) {
		if (WindowShouldClose()) break;

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

		if (IsKeyPressed(KEY_F)) ToggleFullscreen();
		if (IsKeyPressed(KEY_ONE)) renderMode = RenderMode::MODE2D;
		if (IsKeyPressed(KEY_TWO)) renderMode = RenderMode::MODE3D;
		if (IsKeyPressed(KEY_P)) postProcessingSystem.togglePostprocessing();

		// fresh context each frame
		FrameContext ctx;
		ctx.arena       = &arena;
		ctx.gridWidth   = GRID_W;
		ctx.gridHeight  = GRID_H;
		ctx.renderMode  = &renderMode;
		ctx.playerDied  = false;
		renderSystem.fillContext(ctx);  // populate arenaBounds + cellSize before any system reads them
		
		// update phase
		inputSystem.update(registry);
		aiSystem.update(registry, ctx);
		movementSystem.update(registry, dt);
		collisionSystem.update(registry, ruleTable, dispatcher, ctx);
		particleSystem.update(dt, registry, ctx);

		if (ctx.playerDied) {
			std::cout << "PLAYER DIED" << std::endl;
			break;
		}

		// render phase
		postProcessingSystem.beginCapture();
		if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D) {
			renderSystem.beginMode2D();
			particleSystem.render();                       // particles first → under arena/snake
			renderSystem.render2D_content(registry, ctx); // arena, food, snakes on top
			renderSystem.endMode2D();
		} else {
			renderSystem.render(registry, dt, ctx);        // 3D mode (manages its own bracket)
		}
		postProcessingSystem.endCapture();

		// post processing phase
		BeginDrawing();
		ClearBackground(customBlack);
		postProcessingSystem.applyAndPresent(dt);
		DrawFPS(SCREEN_W - 95, 10);
		EndDrawing();
	}

	postProcessingSystem.shutdown();
	CloseWindow();
	return 0;
}