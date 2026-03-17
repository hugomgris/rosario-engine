#pragma once

#include <raylib.h>
#include "../ecs/Registry.hpp"
#include "../components/ButtonComponent.hpp"
#include "../components/UIInteractableComponent.hpp"
#include "../components/ButtonActionComponent.hpp"
#include "../ui/EventQueue.hpp"

class UIInteractionSystem {
public:
    UIInteractionSystem() = default;

    void update(Registry& registry, EventQueue& eventQueue) {
        Vector2 mousePos = GetMousePosition();
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        auto buttonView = registry.view<ButtonComponent, UIInteractableComponent, ButtonActionComponent>();

        bool clickHandled = false;

        for (Entity entity : buttonView) {
            auto& button = registry.getComponent<ButtonComponent>(entity);
            auto& action = registry.getComponent<ButtonActionComponent>(entity);

            button.hovered = CheckCollisionPointRec(mousePos, button.bounds);

            if (mouseClicked && button.hovered && !clickHandled) {
                GameEvent event;
                event.type = GameEvent::Type::ButtonClicked;
                event.buttonAction = action.actionType;
                eventQueue.enqueue(event);
                clickHandled = true;
            }
        }
    }
};
