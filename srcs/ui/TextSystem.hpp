#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <raylib.h>
#include "UIQueue.hpp"

class TextSystem {
    private:
        Font    _font;
        bool    _loaded = false;
        mutable std::unordered_map<std::string, Font> _fontCache;

        void loadFont();
        Font loadFontFromPath(const std::string& path) const;
        const Font& getFontForPath(const std::string& path) const;

        //void drawRects(); // TODO: logo-drawing pipeline

    public:
        void init(); //call AFTER InitWindow()
        void shutdown(); // call BEFORE CloseWindow()

        Font& getFont();

        int measureText(const std::string& text, float fontSize) const;
        void drawText(const std::string& text, float x, float y,
                float fontSize, Color color, bool centered = false) const;
        void drawTextWithFont(const Font& font, const std::string& text, float x, float y,
            float fontSize, Color color, bool centered = false) const;

        //void drawLogo();
        //void drawGameOverLogo();
        void render(UIRenderQueue &queue) const;
};