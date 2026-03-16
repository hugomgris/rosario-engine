#pragma once

#include <string>
#include <vector>
#include <raylib.h>

class TextSystem {
private:
    Font    _font;
    bool    _loaded = false;

    void loadFont();

    // pixel-art rect helpers
    void drawRects(const std::vector<Rectangle>& rects, Color color) const;

public:
    TextSystem()  = default;
    ~TextSystem();

    // Must be called after InitWindow
    void init();

    Font& getFont();

    int  measureText(const std::string& text, float fontSize) const;
    void drawText(const std::string& text, float x, float y,
                  float fontSize, Color color, bool centered = false) const;

    // Pixel-art logo drawers (pending replacement with square-font pipeline)
    void drawLogo(int centerX, int centerY, int square, int sep,
                  Color whiteColor, Color blueColor, Color redColor) const;

    void drawGameOverLogo(int centerX, int centerY, int square, int sep,
                          Color whiteColor, Color grayColor) const;
};
