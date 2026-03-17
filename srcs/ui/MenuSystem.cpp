#include "MenuSystem.hpp"
#include "../../incs/RaylibColors.hpp"
#include "../ecs/Registry.hpp"
#include "../systems/InputSystem.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../helpers/GameManager.hpp"

MenuSystem::MenuSystem(int sWidth, int sHeight) : _screenWidth(sWidth), _screenHeight(sHeight) {
	//setupStartButtons(table.start);
	//setupGameOverButtons(table.gameOver);
}

void MenuSystem::setupButtons(const ButtonConfigLoader::MenuButtonTable& table, std::vector<Button>& target, const MenuContext& ctx) {
	target.clear();
	target.reserve(table.size());

	for (const auto & [ key, value ] : table) {
		(void)key;
		Button button;
		button.config = value;
		button.bounds = {
		(static_cast<float>(_screenWidth) / 2) - (value.width / 2),
		(static_cast<float>(_screenHeight) / 2) - (value.height / 2) + (value.index * value.verticalSpacing),
		value.width,
		value.height
		};

		button.onClick = resolveAction(button.config.id, ctx);

		target.push_back(button);
	}
}

void MenuSystem::setupStartButtons(const ButtonConfigLoader::MenuButtonTable& table, const MenuContext& ctx) {
	setupButtons(table, _startButtons, ctx);
}

void MenuSystem::setupGameOverButtons(const ButtonConfigLoader::MenuButtonTable& table, const MenuContext& ctx) {
	setupButtons(table, _gameOverButtons, ctx);
}

void MenuSystem::enqueueButtons(const std::vector<Button>& buttons, UIRenderQueue& queue) const {
	for (const auto& button : buttons) {
		button.enqueueRenderCommands(queue);
	}
}

void MenuSystem::buildStartMenuUI(FrameContext &ctx, UIRenderQueue& queue) {
	(void)ctx;
	enqueueButtons(_startButtons, queue);
}

void MenuSystem::buildGameOverUI(FrameContext &ctx, UIRenderQueue& queue) {
	(void)ctx;
	enqueueButtons(_gameOverButtons, queue);
}

void MenuSystem::update(GameState& state) {
	Vector2 mousePos = GetMousePosition();
	bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

	auto updateButtons = [&](std::vector<Button>& buttons) {
		for (auto& button : buttons) {
			button.hovered = CheckCollisionPointRec(mousePos, button.bounds);

			if (clicked && button.hovered && button.onClick) {
				button.onClick();
				break; // only one button per click
			}
		}
	};

	if (state == GameState::Menu) {
		updateButtons(_startButtons);
	} else if (state == GameState::GameOver) {
		updateButtons(_gameOverButtons);
	}
}

// Button function hookup
std::function<void()> MenuSystem::resolveAction(const std::string& id, const MenuContext& ctx) {
	if (id == "start") {
		return [ctx]() { *ctx.state = GameState::Playing; };
	} else if (id == "startQuit" || id == "gameoverQuit") {
		return [ctx]() { *ctx.state = GameState::Exiting; };
	} else if (id == "modeChange") {
		return [ctx]() { 
			// Cycle through modes: SINGLE -> MULTI -> VSAI -> SINGLE
			switch (*ctx.mode) {
				case GameMode::SINGLE:
					*ctx.mode = GameMode::MULTI;
					break;
				case GameMode::MULTI:
					*ctx.mode = GameMode::VSAI;
					break;
				case GameMode::VSAI:
					*ctx.mode = GameMode::SINGLE;
					break;
			}
		};
	} else if (id == "returnToMainMenu") {
		return [ctx]() { 
			if (ctx.registry && ctx.inputSystem && ctx.playerSnake && ctx.secondSnake && ctx.food && ctx.arena && ctx.AIPresets) {
				const AIPresetLoader::PresetTable* AIPresets = static_cast<const AIPresetLoader::PresetTable*>(ctx.AIPresets);
				GameManager::resetGame(*ctx.registry, *ctx.inputSystem, *ctx.playerSnake, *ctx.secondSnake, *ctx.food, 
					ctx.gridWidth, ctx.gridHeight, *ctx.arena, *AIPresets, *ctx.mode);
			}
			*ctx.state = GameState::Menu;
		};
	}
	return nullptr;
}