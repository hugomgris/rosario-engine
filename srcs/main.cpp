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
#include "components/ButtonComponent.hpp"
#include "components/ButtonActionComponent.hpp"
#include "systems/InputSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/PostProcessingSystem.hpp"
#include "systems/UIInteractionSystem.hpp"
#include "ui/MenuSystem.hpp"
#include "ui/TextSystem.hpp"
#include "ui/UISystem.hpp"
#include "ui/EventQueue.hpp"
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "helpers/Factories.hpp"
#include "helpers/GameManager.hpp"
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
#include "ui/ButtonConfigLoader.hpp"
#include "ui/UIQueue.hpp"

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

static constexpr int MENU_W = 60;
static constexpr int MENU_H = 33;

// TODO: move this somewhere else, I don't like it here in main
namespace {
	struct TransitionContext {
		ParticleSystem& particleSystem;
		MenuSystem& menuSystem;
		Registry& registry;
		const ButtonConfigLoader::ButtonTable& menuButtons;
		InputSystem& inputSystem;
		Entity& playerSnake;
		Entity& secondSnake;
		Entity& food;
		ArenaGrid& arena;
		const AIPresetLoader::PresetTable& AIPresets;
		GameMode& mode;
	};

	void applyStateTransitionEffects(
		GameState& previousState,
		GameState currentState,
		TransitionContext& ctx
	) {
		ctx.particleSystem.handleStateTransition(previousState, currentState);

		if (currentState == previousState) {
			return;
		}

		switch (currentState) {
			case GameState::Menu:
				ctx.arena.setMenuArena();
				ctx.menuSystem.setupStartButtons(ctx.registry, ctx.menuButtons.start);
				break;
			case GameState::Playing:
				ctx.arena.setGameplayArena();
				GameManager::resetGame(ctx.registry, ctx.inputSystem, ctx.playerSnake, ctx.secondSnake, ctx.food, GRID_W, GRID_H, ctx.arena, ctx.AIPresets, ctx.mode);
				break;
			case GameState::GameOver:
				ctx.arena.setMenuArena();
				ctx.menuSystem.setupGameOverButtons(ctx.registry, ctx.menuButtons.gameOver);
				break;
			case GameState::Paused:
			case GameState::Exiting:
				break;
		}

		previousState = currentState;
	}
}

int main() {
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	// Config json load
	CollisionRuleTable ruleTable = CollisionRuleLoader::load("data/collisionRules.json");
	AIPresetLoader::PresetTable AIPresets = AIPresetLoader::load("data/AIPresets.json");
	ParticleConfig particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json");
	PostProcessConfigLoader::PresetTable ppPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json");
	TunnelConfigLoader::PresetTable tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json");
	std::vector<WallPreset> arenaPresetList = ArenaPresetLoader::load("data/ArenaPresets.json");
	
	ButtonConfigLoader::ButtonTable menuButtons = ButtonConfigLoader::load("data/ButtonConfig.json");

	// Dispatcher set up
	CollisionEffectDispatcher dispatcher;
	dispatcher.registerDefaults();

	// ECS core
	Registry		registry;
	UIRenderQueue	uiQueue;
	GameState		state = GameState::Menu;
	GameMode		mode = GameMode::SINGLE;

	// Gameplay systems
	InputSystem				inputSystem;
	MovementSystem			movementSystem;
	CollisionSystem			collisionSystem;
	AISystem				aiSystem(GRID_W, GRID_H);
	RenderSystem            renderSystem;

	renderSystem.init(GRID_W, GRID_H);

	// UI systems
	MenuSystem	menuSystem(SCREEN_W, SCREEN_H);
	TextSystem	textSystem;
	UISystem	uiSystem;

	textSystem.init();

	// Visual systems
	PostProcessingSystem    postProcessingSystem;
	ParticleSystem          particleSystem(SCREEN_W, SCREEN_H, particleConfig);
	AnimationSystem         animationSystem;

	postProcessingSystem.init(SCREEN_W, SCREEN_H);
	postProcessingSystem.setConfig(ppPresets.at("crt_bloom"));

	// temporary Arena and Context to kickstart the animation system
	{
		ArenaGrid tmpArena(GRID_W, GRID_H);
		FrameContext tmpCtx;
		tmpCtx.arena = &tmpArena;
		tmpCtx.state = &state;
		tmpCtx.gridWidth = GRID_W;
		tmpCtx.gridHeight = GRID_H;
		renderSystem.fillContext(tmpCtx, nullptr);
		animationSystem.init(SCREEN_W, SCREEN_H,
			static_cast<int>(tmpCtx.arenaBounds.x),
			static_cast<int>(tmpCtx.arenaBounds.y),
			tmpCtx.cellSize);
	}
	animationSystem.enable(true, tunnelPresets.at("realm2D"));

	// initial world
	ArenaGrid arena(GRID_W, GRID_H);
	arena.setMenuArena();
	Entity playerSnake(0u), secondSnake(0u), food(0u);
	RenderMode renderMode = RenderMode::MODE2D;
	
	int currentPresetIndex = -1; // -1 = empty arena TODO: think about where to handle this

	// Setup UI systems
	UIInteractionSystem uiInteractionSystem;
	EventQueue eventQueue;

	menuSystem.setupStartButtons(registry, menuButtons.start);
	menuSystem.setupGameOverButtons(registry, menuButtons.gameOver);

	TransitionContext transitionContext {
		particleSystem,
		menuSystem,
		registry,
		menuButtons,
		inputSystem,
		playerSnake,
		secondSnake,
		food,
		arena,
		AIPresets,
		mode
	};

	GameState previousState = state;

	while (true) {
		if (state == GameState::Exiting || WindowShouldClose()) break;

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

		if (IsKeyPressed(KEY_F)) ToggleFullscreen();
		if (IsKeyPressed(KEY_ONE)) renderMode = RenderMode::MODE2D;
		if (IsKeyPressed(KEY_TWO)) renderMode = RenderMode::MODE3D;
		if (IsKeyPressed(KEY_P)) postProcessingSystem.togglePostprocessing();
		if (IsKeyPressed(KEY_TAB)) {
			currentPresetIndex = (currentPresetIndex + 1) % static_cast<int>(arenaPresetList.size());
			animationSystem.notifyArenaSpawning(arena);
			arena.transformArenaWithPreset(arenaPresetList[currentPresetIndex]);
			float lineLifetime = 1.0f / animationSystem.getAnimationSpeed();
			arena.beginSpawn(lineLifetime);
		}

		// fresh context each frame
		FrameContext ctx;
		ctx.arena       = &arena;
		ctx.state		= &state;
		const bool menuLikeState = (state == GameState::Menu || state == GameState::GameOver);
		ctx.gridWidth   = menuLikeState ? MENU_W : GRID_W;
		ctx.gridHeight  = menuLikeState ? MENU_H : GRID_H;
		ctx.renderMode  = &renderMode;
		ctx.playerDied  = false;
		renderSystem.fillContext(ctx, &state);
		animationSystem.init(SCREEN_W, SCREEN_H,
			static_cast<int>(ctx.arenaBounds.x),
			static_cast<int>(ctx.arenaBounds.y),
			ctx.cellSize);

		// UPDATE phase
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				uiInteractionSystem.update(registry, eventQueue);
				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				break;
			
			case GameState::Playing:
				inputSystem.update(registry);
				aiSystem.update(registry, ctx);
				movementSystem.update(registry, dt);
				collisionSystem.update(registry, ruleTable, dispatcher, ctx);
				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				arena.tickSpawnTimer(dt);
				arena.tickDespawnTimer(dt);
				break;

			case GameState::Paused:
				// Todo implement pause
				break;
			
			case GameState::Exiting:
				break;
		}			

		// Death check
		if (ctx.playerDied) {
			std::cout << "PLAYER DIED" << std::endl;
			state = GameState::GameOver;
		}

		// Process button events from UI
		// Maybe there's a better way to code this than 3 nested switches, but hell yeah
		for (const auto& event : eventQueue.getEvents()) {
			switch (event.type) {
				case GameEvent::Type::ButtonClicked:
					switch (event.buttonAction) {
						case ButtonActionType::StartGame:
							state = GameState::Playing;
							break;
						case ButtonActionType::ChangeMode:
							switch (mode) {
								case GameMode::SINGLE:
									mode = GameMode::MULTI;
									break;
								case GameMode::MULTI:
									mode = GameMode::VSAI;
									break;
								case GameMode::VSAI:
									mode = GameMode::SINGLE;
									break;
							}
							break;
						case ButtonActionType::Quit:
							state = GameState::Exiting;
							break;
						case ButtonActionType::ReturnToMenu:
							GameManager::resetGame(registry, inputSystem, playerSnake, secondSnake, food, GRID_W, GRID_H, arena, AIPresets, mode);
							state = GameState::Menu;
							break;
					}
					break;
				default:
					break;
			}
		}
		eventQueue.clear();

		// State transition initialization
		applyStateTransitionEffects(previousState, state, transitionContext);

		// RENDER phase
		postProcessingSystem.beginCapture(); // FOr now, PP affects all states
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				renderSystem.beginMode2D();
				animationSystem.render();
				particleSystem.render();
				renderSystem.renderMenu(ctx);
				renderSystem.endMode2D();
				break;
			
			case GameState::Playing:
				if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D) {
					renderSystem.beginMode2D();
					//animationSystem.update(dt, arena);
					animationSystem.render();
					particleSystem.render();
					renderSystem.render2D(registry, ctx);
					renderSystem.endMode2D();
				} else {
					renderSystem.render(registry, dt, ctx);
				}
				break;
			

			case GameState::Paused:
				// todo pause stuff
				break;
			
			case GameState::Exiting:
				break;
		}

		// UI phase
		uiQueue.clear();
		switch (state) {
			case GameState::Menu:
				menuSystem.buildStartMenuUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				break;
			
			case GameState::Playing:
				// todo game ui
				break;
				
			case GameState::Paused:
				// todo implement puase
				break;

			case GameState::GameOver:
				menuSystem.buildGameOverUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				break;

			case GameState::Exiting:
				break;
		}
		/* uiSystem.renderRects(uiQueue);
		textSystem.render(uiQueue); */
		
		// end pp capture
		postProcessingSystem.endCapture();

		// POST PROCESING phase (PRESENT)
		BeginDrawing();
		ClearBackground(customBlack);
		postProcessingSystem.applyAndPresent(dt);
		DrawFPS(SCREEN_W - 95, 10);
		EndDrawing();
	}

	postProcessingSystem.shutdown();
	textSystem.shutdown();
	CloseWindow();
	return 0;
}