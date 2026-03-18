#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "ButtonConfigLoader.hpp"
#include "../components/ButtonComponent.hpp"
#include "../components/UIRenderableComponent.hpp"
#include "../components/UIInteractableComponent.hpp"
#include "../components/ButtonActionComponent.hpp"

class ButtonFactory {
public:
    static std::vector<Entity> spawnButtonsFromConfig(
        Registry& registry,
        const ButtonConfigLoader::MenuButtonTable& buttonConfigs,
        float screenWidth,
        float screenHeight) {
        
        std::vector<Entity> createdButtons;

        for (const auto& [configId, buttonConfig] : buttonConfigs) {
            Entity buttonEntity = registry.createEntity();
            
            ButtonComponent button;
            button.id = buttonConfig.id;
            button.config = buttonConfig;
            button.bounds = {
                (screenWidth / 2.0f) - (buttonConfig.width / 2.0f),
                (screenHeight * button.config.verticalPositionFactor) - (buttonConfig.height / 2.0f) + (buttonConfig.index * buttonConfig.verticalSpacing),
                buttonConfig.width,
                buttonConfig.height
            };
            registry.addComponent(buttonEntity, button);
            
            registry.addComponent(buttonEntity, UIRenderableComponent{});
            
            registry.addComponent(buttonEntity, UIInteractableComponent{});

            ButtonActionComponent action = resolveActionType(configId);
            registry.addComponent(buttonEntity, action);
            
            createdButtons.push_back(buttonEntity);
        }

        return createdButtons;
    }

private:
    static ButtonActionComponent resolveActionType(const std::string& buttonId) {
        if (buttonId == "start") {
            return ButtonActionComponent{ ButtonActionType::StartGame };
        } else if (buttonId == "modeChange") {
            return ButtonActionComponent{ ButtonActionType::ChangeMode };
        } else if (buttonId == "startQuit" || buttonId == "gameoverQuit") {
            return ButtonActionComponent{ ButtonActionType::Quit };
        } else if (buttonId == "returnToMainMenu") {
            return ButtonActionComponent{ ButtonActionType::ReturnToMenu };
        }
        return ButtonActionComponent{ ButtonActionType::Quit };
    }
};
