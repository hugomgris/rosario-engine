#pragma once

#include <string>

enum class ButtonActionType {
    StartGame,
    ChangeMode,
    Quit,
    ReturnToMenu
};

struct ButtonActionComponent {
    ButtonActionType actionType;
};
