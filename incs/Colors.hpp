#pragma once

struct BaseColor {
    unsigned char r;    // Color red value
    unsigned char g;    // Color green value
    unsigned char b;    // Color blue value
    unsigned char a;    // Color alpha value
};

struct SetColors {
    BaseColor customWhite = { 255, 248, 227, 255};
    BaseColor customGray = { 125, 125, 125, 255};
    BaseColor customBlack = { 23, 23, 23, 255};

    BaseColor wallColor = { 147, 112, 219, 255 };
    BaseColor wallColorFade = { 147, 112, 219, 120 };

    BaseColor FoodColor = {254, 74, 81, 255};

    BaseColor snakeAColor = { 135, 206, 250, 255 };
    BaseColor snakeBColor = { 218, 112, 21, 255 };
    BaseColor snakeAIColor = { 46, 179, 113, 255 };
};