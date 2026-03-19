#pragma once

#include <optional>
#include <vector>
#include <raylib.h>
#include "../ecs/Registry.hpp"
#include "../components/ButtonComponent.hpp"
#include "../components/UIInteractableComponent.hpp"
#include "../components/ButtonActionComponent.hpp"
#include "../ui/EventQueue.hpp"

class UIInteractionSystem {
	private:
		std::optional<Entity> _hoveredButton;
		Vector2 _lastMousePos = {0.0f, 0.0f};
		bool _mouseInitialized = false;

		void enqueueButtonClickEvent(Registry& registry, EventQueue& eventQueue, Entity entity);
		void clearInactiveMenuHover(Registry& registry, ButtonMenu activeMenu);
		void setHoveredEntity(Registry& registry, const std::vector<Entity>& ordered, std::optional<Entity> entity);
		std::optional<size_t> hoveredIndexIn(const std::vector<Entity>& ordered) const;
		std::optional<Entity> pickByMouse(Registry& registry, const std::vector<Entity>& ordered, Vector2 mousePos);
		std::optional<Entity> findButtonById(Registry& registry, const std::vector<Entity>& ordered, const std::string& buttonId) const;

	public:
		UIInteractionSystem() = default;

		void update(Registry& registry, EventQueue& eventQueue, ButtonMenu activeMenu);
		std::vector<Entity> getOrderedButtonsForMenu(Registry& registry, ButtonMenu activeMenu);
};
