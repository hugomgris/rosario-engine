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
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "helpers/Factories.hpp"
#include "helpers/GameState.hpp"
#include "collision/CollisionRule.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "collision/CollisionEffectDispatcher.hpp"
#include "../incs/FrameContext.hpp"
#include "AI/AIPresetLoader.hpp"
#include "systems/AISystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "systems/AnimationSystem.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "ui/TextSystem.hpp"
#include "ui/MenuSystem.hpp"

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
	PostProcessConfigLoader::PresetTable ppPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json");
	TunnelConfigLoader::PresetTable tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json");
	std::vector<WallPreset> arenaPresetList = ArenaPresetLoader::load("data/ArenaPresets.json");

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
	AnimationSystem         animationSystem;

	// UI systems
	TextSystem              textSystem;
	MenuSystem              menuSystem(SCREEN_W, SCREEN_H);

	renderSystem.init(GRID_W, GRID_H);
	postProcessingSystem.init(SCREEN_W, SCREEN_H);
	postProcessingSystem.setConfig(ppPresets.at("crt_bloom"));
	textSystem.init();

	{
		ArenaGrid tmpArena(GRID_W, GRID_H);
		FrameContext tmpCtx;
		tmpCtx.arena = &tmpArena;
		tmpCtx.gridWidth = GRID_W; tmpCtx.gridHeight = GRID_H;
		renderSystem.fillContext(tmpCtx);
		animationSystem.init(SCREEN_W, SCREEN_H,
			static_cast<int>(tmpCtx.arenaBounds.x),
			static_cast<int>(tmpCtx.arenaBounds.y),
			tmpCtx.cellSize);
	}
	animationSystem.enable(true, tunnelPresets.at("realm2D"));

	// Builds a single full-screen rectangle outline in screen space (no scaling needed)
	auto makeFullscreenOutline = [&]() -> std::vector<std::vector<Vector2>> {
		const float m = 12.5f; // half of 25px border thickness — center of the line
		return {{
			{ m,                       m                        },
			{ (float)SCREEN_W - m,     m                        },
			{ (float)SCREEN_W - m,     (float)SCREEN_H - m      },
			{ m,                       (float)SCREEN_H - m      }
		}};
	};

	// App state
	AppState  appState  = AppState::Menu;
	GameMode  gameMode  = GameMode::VSAI;
	RenderMode renderMode = RenderMode::MODE2D;

	ArenaGrid arena(GRID_W, GRID_H);
	Entity playerSnake(0u), secondSnake(0u), food(0u);

	menuSystem.setState(MenuState::Start, gameMode);
	animationSystem.notifyShapeOverride(makeFullscreenOutline());

	bool running = true;
	while (running) {
		if (WindowShouldClose()) break;

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

		// Global keys (always active)
		if (IsKeyPressed(KEY_F)) ToggleFullscreen();
		if (IsKeyPressed(KEY_P)) postProcessingSystem.togglePostprocessing();

		// ---- State machine update ----
		switch (appState) {

			case AppState::Menu:
			case AppState::GameOver: {
				MenuAction action = menuSystem.update(dt, gameMode, particleSystem);
				switch (action) {
			case MenuAction::StartGame:
				particleSystem.clearMenuTrail();
				GameState::resetGame(registry, inputSystem, playerSnake, secondSnake,
									 food, GRID_W, GRID_H, arena, AIPresets, gameMode);
				// Instantly snap tunnel lines to arena shape, clearing fullscreen residue
				animationSystem.instantShapeChange(animationSystem.getArenaOutlines());
				appState = AppState::Playing;
				break;
					case MenuAction::SwitchMode:
						gameMode = static_cast<GameMode>(
							(static_cast<int>(gameMode) + 1) % 3);
						menuSystem.setState(MenuState::Start, gameMode);
						break;
					case MenuAction::Restart:
						particleSystem.clearGameplay();
						animationSystem.notifyShapeOverride(makeFullscreenOutline());
						GameState::resetGame(registry, inputSystem, playerSnake, secondSnake,
											 food, GRID_W, GRID_H, arena, AIPresets, gameMode);
						appState = AppState::Menu;
						menuSystem.setState(MenuState::Start, gameMode);
						break;
					case MenuAction::Quit:
						running = false;
						break;
					default: break;
				}
				break;
			}

			case AppState::Playing: {
				if (IsKeyPressed(KEY_ONE)) renderMode = RenderMode::MODE2D;
				if (IsKeyPressed(KEY_TWO)) renderMode = RenderMode::MODE3D;
				if (IsKeyPressed(KEY_TAB)) {
					static int presetIdx = -1;
					presetIdx = (presetIdx + 1) % static_cast<int>(arenaPresetList.size());
					animationSystem.notifyArenaSpawning(arena);
					arena.transformArenaWithPreset(arenaPresetList[presetIdx]);
					arena.beginSpawn(1.0f / animationSystem.getAnimationSpeed());
				}

				FrameContext ctx;
				ctx.arena      = &arena;
				ctx.gridWidth  = GRID_W;
				ctx.gridHeight = GRID_H;
				ctx.renderMode = &renderMode;
				ctx.playerDied = false;
				renderSystem.fillContext(ctx);

				inputSystem.update(registry);
				aiSystem.update(registry, ctx);
				movementSystem.update(registry, dt);
				collisionSystem.update(registry, ruleTable, dispatcher, ctx);
				particleSystem.update(dt, registry, ctx);
				arena.tickSpawnTimer(dt);
				arena.tickDespawnTimer(dt);

				if (ctx.playerDied) {
					particleSystem.clearGameplay();
					particleSystem.clearMenuTrail();
					animationSystem.notifyShapeOverride(makeFullscreenOutline());
					appState = AppState::GameOver;
					menuSystem.setState(MenuState::GameOver, gameMode);
				}
				break;
			}

			case AppState::Paused:
				break;
		}

		// ---- Render phase ----
		postProcessingSystem.beginCapture();
		renderSystem.beginMode2D();

		// Tunnel lines + particles always render (visible in menu and gameplay)
		animationSystem.update(dt, arena);
		animationSystem.render();
		particleSystem.render();

		switch (appState) {
			case AppState::Menu:
				menuSystem.render(textSystem, gameMode);
				break;

			case AppState::Playing:
			case AppState::Paused: {
				FrameContext ctx;
				ctx.arena      = &arena;
				ctx.gridWidth  = GRID_W;
				ctx.gridHeight = GRID_H;
				ctx.renderMode = &renderMode;
				renderSystem.fillContext(ctx);
				if (renderMode == RenderMode::MODE2D)
					renderSystem.render2D(registry, ctx);
				else
					renderSystem.render(registry, dt, ctx);
				break;
			}

			case AppState::GameOver:
				menuSystem.renderGameOver(textSystem, gameMode);
				break;
		}

		renderSystem.endMode2D();
		postProcessingSystem.endCapture();

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
