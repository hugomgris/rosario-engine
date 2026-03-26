#include "UIInteractionSystem.hpp"

#include <algorithm>

void UIInteractionSystem::enqueueButtonClickEvent(Registry& registry, EventQueue& eventQueue, Entity entity) {
	const auto& action = registry.getComponent<ButtonActionComponent>(entity);
	GameEvent event;
	event.type = GameEvent::Type::ButtonClicked;
	event.buttonAction = action.actionType;
	eventQueue.enqueue(event);
}

void UIInteractionSystem::clearInactiveMenuHover(Registry& registry, ButtonMenu activeMenu) {
	auto view = registry.view<ButtonComponent, UIInteractableComponent>();
	for (Entity entity : view) {
		auto& button = registry.getComponent<ButtonComponent>(entity);
		if (button.config.menu != activeMenu) {
			button.hovered = false;
		}
	}
}

void UIInteractionSystem::setHoveredEntity(Registry& registry, const std::vector<Entity>& ordered, std::optional<Entity> entity) {
	for (Entity candidate : ordered) {
		auto& button = registry.getComponent<ButtonComponent>(candidate);
		button.hovered = (entity.has_value() && candidate == *entity);
	}
	_hoveredButton = entity;
}

std::optional<size_t> UIInteractionSystem::hoveredIndexIn(const std::vector<Entity>& ordered) const {
	if (!_hoveredButton.has_value()) {
		return std::nullopt;
	}
	for (size_t i = 0; i < ordered.size(); ++i) {
		if (ordered[i] == *_hoveredButton) {
			return i;
		}
	}
	return std::nullopt;
}

std::optional<Entity> UIInteractionSystem::pickByMouse(Registry& registry, const std::vector<Entity>& ordered, Vector2 mousePos) {
	for (Entity entity : ordered) {
		const auto& button = registry.getComponent<ButtonComponent>(entity);
		if (CheckCollisionPointRec(mousePos, button.bounds)) {
			return entity;
		}
	}
	return std::nullopt;
}

std::optional<Entity> UIInteractionSystem::findButtonById(Registry& registry, const std::vector<Entity>& ordered, const std::string& buttonId) const {
	for (Entity entity : ordered) {
		const auto& button = registry.getComponent<ButtonComponent>(entity);
		if (button.id == buttonId) {
			return entity;
		}
	}
	return std::nullopt;
}

void UIInteractionSystem::update(Registry& registry, EventQueue& eventQueue, ButtonMenu activeMenu) {
	auto ordered = getOrderedButtonsForMenu(registry, activeMenu);
	clearInactiveMenuHover(registry, activeMenu);

	if (ordered.empty()) {
		_hoveredButton.reset();
		return;
	}

	if (!hoveredIndexIn(ordered).has_value()) {
		setHoveredEntity(registry, ordered, std::nullopt);
	}

	Vector2 mousePos = GetMousePosition();
	const bool mouseMoved = _mouseInitialized && (mousePos.x != _lastMousePos.x || mousePos.y != _lastMousePos.y);
	_mouseInitialized = true;
	_lastMousePos = mousePos;

	const bool moveDown = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
	const bool moveUp = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
	const bool confirmPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
	const bool modeAliasPressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D);
	const bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

	if (moveDown || moveUp) {
		std::optional<size_t> current = hoveredIndexIn(ordered);
		size_t targetIndex = 0;

		if (!current.has_value()) {
			targetIndex = moveUp ? (ordered.size() - 1) : 0;
		} else if (moveDown) {
			targetIndex = (*current + 1) % ordered.size();
		} else {
			targetIndex = (*current + ordered.size() - 1) % ordered.size();
		}

		setHoveredEntity(registry, ordered, ordered[targetIndex]);
	}

	if (activeMenu == ButtonMenu::Start && modeAliasPressed) {
		auto modeButton = findButtonById(registry, ordered, "modeChange");
		if (modeButton.has_value()) {
			setHoveredEntity(registry, ordered, modeButton);
			enqueueButtonClickEvent(registry, eventQueue, *modeButton);
		}
	}

	std::optional<Entity> mouseCandidate;
	if (mouseMoved || mouseClicked) {
		mouseCandidate = pickByMouse(registry, ordered, mousePos);
		if (mouseCandidate.has_value()) {
			setHoveredEntity(registry, ordered, mouseCandidate);
		}
	}

	if (mouseClicked && mouseCandidate.has_value()) {
		enqueueButtonClickEvent(registry, eventQueue, *mouseCandidate);
	} else if (confirmPressed && _hoveredButton.has_value()) {
		enqueueButtonClickEvent(registry, eventQueue, *_hoveredButton);
	}
}

std::vector<Entity> UIInteractionSystem::getOrderedButtonsForMenu(Registry& registry, ButtonMenu activeMenu) {
	auto view = registry.view<ButtonComponent, UIInteractableComponent, ButtonActionComponent>();
	std::vector<Entity> ordered;
	ordered.reserve(view.size());

	for (Entity e : view) {
		const auto& button = registry.getComponent<ButtonComponent>(e);
		if (button.config.menu == activeMenu) {
			ordered.push_back(e);
		}
	}

	std::sort(ordered.begin(), ordered.end(), [&](Entity a, Entity b) {
		const auto& ba = registry.getComponent<ButtonComponent>(a);
		const auto& bb = registry.getComponent<ButtonComponent>(b);
		return ba.config.index < bb.config.index;
	});

	return ordered;
}

void UIInteractionSystem::setHoveredForTests(Registry& registry, ButtonMenu activeMenu, std::optional<Entity> entity) {
	auto ordered = getOrderedButtonsForMenu(registry, activeMenu);
	clearInactiveMenuHover(registry, activeMenu);

	if (ordered.empty()) {
		_hoveredButton.reset();
		return;
	}

	if (entity.has_value()) {
		auto it = std::find(ordered.begin(), ordered.end(), *entity);
		if (it == ordered.end()) {
			setHoveredEntity(registry, ordered, std::nullopt);
			return;
		}
	}

	setHoveredEntity(registry, ordered, entity);
}
