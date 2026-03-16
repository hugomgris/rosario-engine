#pragma once

#include <string>
#include <vector>
#include <raylib.h>
#include "UIQueue.hpp"

class TextSystem {
    private:
        Font    _font;
        bool    _loaded = false;

        void loadFont();

        //void drawRects(); // TODO: logo-drawing pipeline

    public:
        TextSystem() = default;
        ~TextSystem();

        void init(); //call AFTER InitWindow()

        Font& getFont();

        int measureText(const std::string& text, float fontSize) const;
        void drawText(const std::string& text, float x, float y,
                float fontSize, Color color, bool centered = false) const;

        //void drawLogo();
        //void drawGameOverLogo();
        void render(UIRenderQueue &queue) const;
};