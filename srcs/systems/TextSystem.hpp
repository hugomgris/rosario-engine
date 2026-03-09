#pragma once
#include <string>
#include <vector>
#include <raylib.h>

// ─── TextSystem ───────────────────────────────────────────────────────────────
//
// Owns a JetBrains Mono custom Font and exposes helpers for drawing
// the pixel-art logos and HUD text that the old OOP version used.
//
// Must be initialised after InitWindow() (font loading requires an open GL context).
//
class TextSystem {
public:
    TextSystem();
    ~TextSystem();

    void init();
    Font& getFont();
    int   measureText(const std::string& text, int fontSize);

    // ── Pixel-art logo renders ─────────────────────────────────────────────
    void drawLogo        (int centerX, int centerY, int square, int sep,
                          Color whiteColor, Color blueColor, Color redColor);
    void drawGameOverLogo(int centerX, int centerY, int square, int sep,
                          Color whiteColor, Color grayColor);

    // ── General text helpers ───────────────────────────────────────────────
    void drawText(const std::string& text, int x, int y, int fontSize,
                  Color color, bool centered = false);

private:
    Font customFont;
    int  fontSize      = 30;
    int  smallFontSize = 20;
    bool smallMode     = false;

    void loadFont();
    void drawRects(const std::vector<Rectangle>& rects, Color color);
};
