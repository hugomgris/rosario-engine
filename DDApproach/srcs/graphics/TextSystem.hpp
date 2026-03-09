#pragma once

#include <string>
#include <vector>
#include "Renderer.hpp"

class TextSystem {
public:
    TextSystem(const Renderer& renderer);
    void renderText(const std::string& text, float x, float y, float scale, const Color& color);
    void loadFont(const std::string& fontPath, int fontSize);
    void unloadFont();

private:
    Renderer renderer;
    Font font;
};