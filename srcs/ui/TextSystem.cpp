#include <iostream>
#include "TextSystem.hpp"

TextSystem::~TextSystem() {
    UnloadFont(_font);
}

void TextSystem::init() {
    loadFont();
}

void TextSystem::loadFont() {
    int     codepointCount = 256 - 32 + 8;
    int*    codepoints = new int[codepointCount];

    for (int i = 0; i < (256 - 32); i++) {
        codepoints[i] = 32 + i;
    }

    codepoints[256 - 32 + 0] = 0x2191;
    codepoints[256 - 32 + 1] = 0x2193;
    codepoints[256 - 32 + 2] = 0x2190;
    codepoints[256 - 32 + 3] = 0x2192;
    codepoints[256 - 32 + 4] = 0x00B7;
    codepoints[256 - 32 + 5] = 0x2022;
    codepoints[256 - 32 + 6] = 0x00B7;
    codepoints[256 - 32 + 7] = 0x002E;

    _font = LoadFontEx("fonts/JetBrainsMono-VariableFont_wght.ttf", 64, codepoints, codepointCount);
    delete[] codepoints;
}

int TextSystem::measureText(const std::string& text, float fontSize) const {
    Vector2 textSize = MeasureTextEx(_font, text.c_str(), fontSize, 1.0f);
    return static_cast<int>(textSize.x);
}

void TextSystem::drawText(const std::string& text, float x, float y,
                float fontSize, Color color, bool centered) const {
    Vector2 textSize = MeasureTextEx(_font, text.c_str(), fontSize, 1.0f);
    int drawX = centered ? x - (textSize.x / 2) : x;
    int drawY = y - (textSize.y / 2);

    DrawTextEx(_font, text.c_str(), (Vector2){(float)drawX, (float)drawY}, fontSize, 1.0f, color);
}

Font& TextSystem::getFont() { return _font; }

void TextSystem::render(UIRenderQueue &queue) const {
    for (UITextCmd cmd : queue.texts) {
        drawText(cmd.text, cmd.x, cmd.y, cmd.fontSize, cmd.color, cmd.centered);
    }
}