#pragma once

#include <string>
#include <vector>
#include <raylib.h>

struct UIRectCmd {
    Rectangle   rect;
    Color       color;
    bool        outline = false;
    float       lineThickness = 1.0f;
};

struct UITextCmd {
    std::string text;
    float   x;
    float   y;
    float   fontSize;
    Color   color;
    bool    centered = false;
};

struct UIRenderQueue {
    std::vector<UIRectCmd>  rects;
    std::vector<UITextCmd>  texts;

    void clear () {
        rects.clear();
        texts.clear();
    }
};