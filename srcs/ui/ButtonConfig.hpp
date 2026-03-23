#pragma once

#include <string>
#include <raylib.h>

enum class ButtonMenu {
    Start,
    GameOver
};

struct ButtonConfig {
    std::string     id;
    ButtonMenu      menu = ButtonMenu::Start;
    std::string     label;
    int             index;
    float           height;
    float           width;
    bool            hasX = false;
    bool            hasY = false;
    float           x = 0.0f;
    float           y = 0.0f;
    bool            centerX = false;
    bool            centerY = false;
    float           verticalPositionFactor;
    float           fontSize = 24.0f;
    std::string     fontPath;
    float           outlineThickness = 2.0f;
    float           verticalSpacing = 100.0f;
    Color           outlineColor;
    Color           backgroundColor;
    Color           textColor;
    Color           hoverColor;
    Color           outlineHoverColor;
    Color           textHoverColor;
};