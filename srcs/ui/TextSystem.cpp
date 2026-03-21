#include <iostream>
#include "TextSystem.hpp"

void TextSystem::init() {
    loadFont();
}

void TextSystem::shutdown() {
    for (auto& [_, font] : _fontCache) {
        if (font.texture.id != 0) {
            UnloadFont(font);
        }
    }
    _fontCache.clear();

    if (_loaded) {
        UnloadFont(_font);
        _font = {};
        _loaded = false;
    }
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
    _loaded = true;
}

int TextSystem::measureText(const std::string& text, float fontSize) const {
    Vector2 textSize = MeasureTextEx(_font, text.c_str(), fontSize, 1.0f);
    return static_cast<int>(textSize.x);
}

Font TextSystem::loadFontFromPath(const std::string& path) const {
    if (path.empty()) {
        return _font;
    }

    Font font = LoadFontEx(path.c_str(), 64, nullptr, 0);
    if (font.texture.id == 0) {
        std::cerr << "TextSystem: failed to load font from path: " << path << std::endl;
        return _font;
    }
    return font;
}

const Font& TextSystem::getFontForPath(const std::string& path) const {
    if (path.empty()) {
        return _font;
    }

    auto it = _fontCache.find(path);
    if (it != _fontCache.end()) {
        return it->second;
    }

    Font loaded = loadFontFromPath(path);
    if (loaded.texture.id == 0 || loaded.texture.id == _font.texture.id) {
        return _font;
    }

    auto [insertedIt, _] = _fontCache.emplace(path, loaded);
    return insertedIt->second;
}

void TextSystem::drawTextWithFont(const Font& font, const std::string& text, float x, float y,
                            float fontSize, Color color, bool centered) const {
    Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, 1.0f);
    int drawX = centered ? x - (textSize.x / 2) : x;
    int drawY = y - (textSize.y / 2);

    DrawTextEx(font, text.c_str(), (Vector2){(float)drawX, (float)drawY}, fontSize, 1.0f, color);
}

void TextSystem::drawText(const std::string& text, float x, float y,
                float fontSize, Color color, bool centered) const {
    drawTextWithFont(_font, text, x, y, fontSize, color, centered);
}

Font& TextSystem::getFont() { return _font; }

void TextSystem::render(UIRenderQueue &queue) const {
    for (const UITextCmd& cmd : queue.texts) {
		const Font& font = getFontForPath(cmd.fontPath);
		drawTextWithFont(font, cmd.text, cmd.x, cmd.y, cmd.fontSize, cmd.color, cmd.centered);
    }
}