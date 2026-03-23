#include "MenuSystem.hpp"
#include "../../incs/RaylibColors.hpp"
#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/ButtonComponent.hpp"
#include "../components/UIRenderableComponent.hpp"
#include <algorithm>

MenuSystem::MenuSystem(int sWidth, int sHeight) : _screenWidth(sWidth), _screenHeight(sHeight) {
}

void MenuSystem::setupStartButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table) {
	clearStartButtons(registry);
	
	_startButtonEntities = ButtonFactory::spawnButtonsFromConfig(registry, table, _screenWidth, _screenHeight);
}

void MenuSystem::setupGameOverButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table) {
	clearGameOverButtons(registry);
	
	_gameOverButtonEntities = ButtonFactory::spawnButtonsFromConfig(registry, table, _screenWidth, _screenHeight);
}

void MenuSystem::clearStartButtons(Registry& registry) {
	for (Entity entity : _startButtonEntities) {
		registry.destroyEntity(entity);
	}
	_startButtonEntities.clear();
}

void MenuSystem::clearGameOverButtons(Registry& registry) {
	for (Entity entity : _gameOverButtonEntities) {
		registry.destroyEntity(entity);
	}
	_gameOverButtonEntities.clear();
}

void MenuSystem::buildStartMenuUI(Registry& registry, UIRenderQueue& queue) {
	auto buttonView = registry.view<ButtonComponent, UIRenderableComponent>();
	
	for (Entity entity : buttonView) {
		auto it = std::find(_startButtonEntities.begin(), _startButtonEntities.end(), entity);
		if (it != _startButtonEntities.end()) {
			auto& button = registry.getComponent<ButtonComponent>(entity);
			enqueueButtonRenderCommands(button, queue);
		}
	}
}

void MenuSystem::buildGameOverUI(Registry& registry, UIRenderQueue& queue) {
	auto buttonView = registry.view<ButtonComponent, UIRenderableComponent>();
	
	for (Entity entity : buttonView) {
		auto it = std::find(_gameOverButtonEntities.begin(), _gameOverButtonEntities.end(), entity);
		if (it != _gameOverButtonEntities.end()) {
			auto& button = registry.getComponent<ButtonComponent>(entity);
			enqueueButtonRenderCommands(button, queue);
		}
	}
}

void MenuSystem::enqueueButtonRenderCommands(const ButtonComponent& button, UIRenderQueue& queue) const {
	Color bg		= button.hovered ? button.config.hoverColor : button.config.backgroundColor;
	Color outline	= button.hovered ? button.config.outlineHoverColor : button.config.outlineColor;
	Color text		= button.hovered ? button.config.textHoverColor  : button.config.textColor;

	queue.rects.push_back(UIRectCmd{
		button.bounds,
		bg,
		false,
		button.config.outlineThickness
	});

	queue.rects.push_back(UIRectCmd{
		button.bounds,
		outline,
		true,
		button.config.outlineThickness
	});

	queue.texts.push_back(UITextCmd{
		button.config.label,
		button.bounds.x + button.bounds.width * 0.5f,
		button.bounds.y + button.bounds.height * 0.5f,
		button.config.fontSize,
		text,
		button.config.fontPath,
		true
	});
}