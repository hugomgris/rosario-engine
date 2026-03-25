#include <iostream>
#include <algorithm>
#include <ctime>
#include <limits>
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
#include "components/ParticleSpawnRequest.hpp"
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
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
#include "ui/GlyphLibraryLoader.hpp"
#include "ui/GlyphPresetLoader.hpp"
#include "ui/PixelTextLayoutSystem.hpp"
#include "ui/PixelTextRenderSystem.hpp"
#include "ui/PixelTextHelper.hpp"
#include "helpers/StateTransitionHelper.hpp"
#include "helpers/MenuLogoParticleHelper.hpp"

// constants
static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

static constexpr int GRID_W = 32;
static constexpr int GRID_H = 32;

static constexpr int MENU_W = 60;
static constexpr int MENU_H = 33;

int main() {
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	// Config json load
	CollisionRuleTable ruleTable = CollisionRuleLoader::load("data/CollisionRules.json");
	AIPresetLoader::PresetTable AIPresets = AIPresetLoader::load("data/AIPresets.json");
	ParticleConfig particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json");
	PostProcessConfigLoader::PresetTable ppPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json");
	TunnelConfigLoader::PresetTable tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json");
	ArenaPresetLoader::PresetList arenaPresetList = ArenaPresetLoader::load("data/ArenaPresets.json");
	GlyphLibrary glyphLib = GlyphLibraryLoader::load("data/GlyphLibrary.json");
	GlyphPresetLoader::PresetTable glyphPresets = GlyphPresetLoader::load("data/GlyphPresets.json");
	
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
	PixelTextLayoutSystem pixelTextLayoutSystem;
	PixelTextRenderSystem pixelTextRenderSystem;

	textSystem.init();
	PixelTextComponent gameOverTitleTemplate = PixelTextHelper::makeGameOverTitleTemplate(glyphPresets);
	PixelTextComponent menuLogoTemplate;
	bool hasMenuLogoTemplate = PixelTextHelper::makeMenuLogoTemplate(glyphPresets, menuLogoTemplate);

	Entity gameOverTitle = Factories::spawnPixelText(registry, gameOverTitleTemplate, true);
	Entity menuLogo(0u);
	std::vector<Entity> menuTrailEmitters;
	if (hasMenuLogoTemplate) {
		menuLogo = Factories::spawnPixelText(registry, menuLogoTemplate, true);
	}

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
	bool debugLayout = false;
	bool debugParticle = false;
	
	int currentPresetIndex = -1; // -1 = empty arena TODO: think about where to handle this

	// Setup UI systems
	UIInteractionSystem uiInteractionSystem;
	EventQueue eventQueue;

	menuSystem.setupStartButtons(registry, menuButtons.start);
	menuSystem.setupGameOverButtons(registry, menuButtons.gameOver);

	StateTransitionContext transitionContext {
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
		if (IsKeyPressed(KEY_F10)) debugLayout = !debugLayout;
		if (IsKeyPressed(KEY_F8)) debugParticle = !debugParticle;
		if (IsKeyPressed(KEY_F9)) {
			try {
				glyphLib = GlyphLibraryLoader::load("data/GlyphLibrary.json");
				glyphPresets = GlyphPresetLoader::load("data/GlyphPresets.json");

				gameOverTitleTemplate = PixelTextHelper::makeGameOverTitleTemplate(glyphPresets);
				hasMenuLogoTemplate = PixelTextHelper::makeMenuLogoTemplate(glyphPresets, menuLogoTemplate);

				PixelTextHelper::ensurePixelTextEntity(registry, gameOverTitle, gameOverTitleTemplate);
				PixelTextHelper::applyPixelTextTemplate(registry, gameOverTitle, gameOverTitleTemplate);

				if (hasMenuLogoTemplate) {
				PixelTextHelper::ensurePixelTextEntity(registry, menuLogo, menuLogoTemplate);
				PixelTextHelper::applyPixelTextTemplate(registry, menuLogo, menuLogoTemplate);
				} else if (registry.hasComponent<PixelTextComponent>(menuLogo)
					&& registry.hasComponent<PixelTextLayoutComponent>(menuLogo)) {
					auto& text = registry.getComponent<PixelTextComponent>(menuLogo);
					auto& layout = registry.getComponent<PixelTextLayoutComponent>(menuLogo);
					text.visible = false;
					text.visibleInStates.clear();
					layout.dirty = true;
				}

				PixelTextHelper::applyPixelTextStateVisibility(registry, state);
				std::cout << "[GlyphPipeline] Hot reload successful" << std::endl;
			} catch (const std::exception& e) {
				std::cerr << "[GlyphPipeline] Hot reload failed: " << e.what() << std::endl;
			}
		}
		if (IsKeyPressed(KEY_TAB)) {
			currentPresetIndex = (currentPresetIndex + 1) % static_cast<int>(arenaPresetList.size());
			animationSystem.notifyArenaSpawning(arena);
			arena.transformArenaWithPreset(arenaPresetList[currentPresetIndex].walls);
			float lineLifetime = 1.0f / animationSystem.getAnimationSpeed();
			arena.beginSpawn(lineLifetime);
		}

		// 1) Gather UI input for menu-like states (fills eventQueue)
		if (state == GameState::Menu || state == GameState::GameOver) {
			const ButtonMenu activeMenu = (state == GameState::Menu)
				? ButtonMenu::Start
				: ButtonMenu::GameOver;
			uiInteractionSystem.update(registry, eventQueue, activeMenu);
		}

		// 2) Process button events (state/mode mutations only)
		for (const auto& event : eventQueue.getEvents()) {
			switch (event.type) {
				case GameEvent::Type::ButtonClicked:
					switch (event.buttonAction) {
						case ButtonActionType::StartGame:
							state = GameState::Playing;
							break;
						case ButtonActionType::ChangeMode:
							switch (mode) {
								case GameMode::SINGLE: mode = GameMode::MULTI; break;
								case GameMode::MULTI:  mode = GameMode::VSAI;  break;
								case GameMode::VSAI:   mode = GameMode::SINGLE; break;
							}
							break;
						case ButtonActionType::Quit:
							state = GameState::Exiting;
							break;
						case ButtonActionType::ReturnToMenu:
							GameManager::resetGame(registry, inputSystem, playerSnake, secondSnake, food,
												GRID_W, GRID_H, arena, AIPresets, mode);
							state = GameState::Menu;
							break;
					}
					break;
				default:
					break;
			}
		}
		eventQueue.clear();

		// 3) Apply transitions BEFORE building context
		const bool transitionedThisFrame = (state != previousState);
		StateTransitionHelper::applyStateTransitionEffects(previousState, state, transitionContext);

		// 4) Build fresh context from current state/arena
		FrameContext ctx;
		ctx.arena      = &arena;
		ctx.state      = &state;
		const bool menuLikeState = (state == GameState::Menu || state == GameState::GameOver);
		ctx.menuLikeFrame = menuLikeState;
		ctx.gridWidth  = menuLikeState ? MENU_W : GRID_W;
		ctx.gridHeight = menuLikeState ? MENU_H : GRID_H;
		ctx.renderMode = &renderMode;
		ctx.playerDied = false;

		renderSystem.fillContext(ctx, &state);
		animationSystem.init(
			SCREEN_W, SCREEN_H,
			static_cast<int>(ctx.arenaBounds.x),
			static_cast<int>(ctx.arenaBounds.y),
			ctx.cellSize
		);

		// 5) UPDATE phase
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				PixelTextHelper::ensurePixelTextEntity(registry, gameOverTitle, gameOverTitleTemplate);

				if (hasMenuLogoTemplate) {
					PixelTextHelper::ensurePixelTextEntity(registry, menuLogo, menuLogoTemplate);
				}

				PixelTextHelper::applyPixelTextStateVisibility(registry, state);
				pixelTextLayoutSystem.update(registry, glyphLib);

				if (hasMenuLogoTemplate) {
					MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, menuLogo, particleConfig, menuTrailEmitters);
				}

				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				break;

			case GameState::Playing:
				if (!transitionedThisFrame) {
					inputSystem.update(registry);
					aiSystem.update(registry, ctx);
					movementSystem.update(registry, dt);
					collisionSystem.update(registry, ruleTable, dispatcher, ctx);
					arena.tickSpawnTimer(dt);
					arena.tickDespawnTimer(dt);
				}
				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				break;

			case GameState::Paused:
				break;

			case GameState::Exiting:
				break;
		}

		// Death check happens after update phase
		if (ctx.playerDied) {
			std::cout << "PLAYER DIED" << std::endl;
			state = GameState::GameOver;
		}

		// 6) RENDER phase
		postProcessingSystem.beginCapture();
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				renderSystem.beginMode2D();
				animationSystem.render();
				particleSystem.render();
				renderSystem.renderMenu(ctx);
				if (debugLayout) renderSystem.renderDebugLayout(ctx);
				renderSystem.endMode2D();
				break;

			case GameState::Playing:
				if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D) {
					renderSystem.beginMode2D();
					animationSystem.render();
					particleSystem.render();
					renderSystem.render2D(registry, ctx);
					if (debugLayout) renderSystem.renderDebugLayout(ctx);
					renderSystem.endMode2D();
				} else {
					renderSystem.render(registry, dt, ctx);
				}
				break;

			case GameState::Paused:
			case GameState::Exiting:
				break;
		}

		// 7) UI phase
		uiQueue.clear();
		switch (state) {
			case GameState::Menu:
				menuSystem.buildStartMenuUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				pixelTextRenderSystem.render(registry);
				break;
			case GameState::GameOver:
				menuSystem.buildGameOverUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				pixelTextRenderSystem.render(registry);
				break;
			case GameState::Playing:
			case GameState::Paused:
			case GameState::Exiting:
				break;
		}

		postProcessingSystem.endCapture();

		// 8) PRESENTING phase (post processing et al)
		BeginDrawing();
		ClearBackground(customBlack);
		postProcessingSystem.applyAndPresent(dt);
		if (debugLayout) DrawFPS(SCREEN_W - 95, 10);
		if (debugParticle) {
			DrawText(
				TextFormat(
					"Particles:%d  MenuTrail:%d  MenuTrailReq:%d",
					static_cast<int>(particleSystem.getParticleCount()),
					static_cast<int>(particleSystem.getMenuTrailParticleCount()),
					static_cast<int>(particleSystem.getLastMenuTrailRequestCount())
				),
				SCREEN_W - 460,
				36,
				20,
				customWhite
			);
		}
		EndDrawing();
	}

	postProcessingSystem.shutdown();
	textSystem.shutdown();
	CloseWindow();
	return 0;
}