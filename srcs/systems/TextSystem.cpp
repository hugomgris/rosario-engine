#include "systems/TextSystem.hpp"
#include <vector>
#include <raymath.h>

// ─── Lifecycle ────────────────────────────────────────────────────────────────

TextSystem::TextSystem() {}

TextSystem::~TextSystem() {
    // IsWindowReady() guard: UnloadFont is a no-op on a zero-init Font
    if (IsWindowReady())
        UnloadFont(customFont);
}

void TextSystem::init() {
    loadFont();
    smallMode = (GetScreenWidth() / 2 < 900);
}

void TextSystem::loadFont() {
    int codepointCount = 256 - 32 + 8;
    int* codepoints = new int[codepointCount];

    for (int i = 0; i < (256 - 32); ++i)
        codepoints[i] = 32 + i;

    // Arrows, bullet, dot
    codepoints[256 - 32 + 0] = 0x2191;
    codepoints[256 - 32 + 1] = 0x2193;
    codepoints[256 - 32 + 2] = 0x2190;
    codepoints[256 - 32 + 3] = 0x2192;
    codepoints[256 - 32 + 4] = 0x00B7;
    codepoints[256 - 32 + 5] = 0x2022;
    codepoints[256 - 32 + 6] = 0x00B7;
    codepoints[256 - 32 + 7] = 0x002E;

    customFont = LoadFontEx("fonts/JetBrainsMono-VariableFont_wght.ttf",
                            64, codepoints, codepointCount);
    delete[] codepoints;
}

Font& TextSystem::getFont() { return customFont; }

int TextSystem::measureText(const std::string& text, int fs) {
    Vector2 v = MeasureTextEx(customFont, text.c_str(), static_cast<float>(fs), 1.0f);
    return static_cast<int>(v.x);
}

// ─── Internal helper ─────────────────────────────────────────────────────────

void TextSystem::drawRects(const std::vector<Rectangle>& rects, Color color) {
    for (const auto& r : rects)
        DrawRectangleRec(r, color);
}

// ─── Pixel-art "nibbler" logo ─────────────────────────────────────────────────

void TextSystem::drawLogo(int centerX, int centerY, int sq, int sep,
                          Color whiteColor, Color blueColor, Color redColor) {
    int totalWidth = (26 * sq) + (6 * sep);
    int startX = centerX - (totalWidth / 2);

    // n
    drawRects({
        { (float)startX,                   (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(startX + sq),            (float)(centerY - sq*3), (float)(sq*3), (float)sq     },
        { (float)(startX + sq*3),          (float)(centerY - sq*2), (float)(sq*2), (float)sq     },
        { (float)(startX + sq*4),          (float)(centerY - sq),   (float)sq,     (float)(sq*3) },
    }, whiteColor);

    // i base
    drawRects({
        { (float)(startX + sq*5 + sep), (float)(centerY - sq*4), (float)sq,    (float)(sq*7) },
        { (float)(startX + sq*5 + sep), (float)(centerY + sq*3), (float)(sq*27),(float)sq    },
    }, blueColor);

    // i dot
    drawRects({
        { (float)(startX + sq*5 + sep), (float)(centerY - sq*6), (float)sq, (float)sq },
    }, redColor);

    // bbler
    int bx = startX + sq*6 + sep*2;
    drawRects({
        // First 'b'
        { (float)bx,              (float)(centerY - sq*6), (float)sq,     (float)(sq*8) },
        { (float)(bx + sq),       (float)(centerY - sq*3), (float)(sq*4), (float)sq     },
        { (float)(bx + sq*4),     (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(bx + sq),       (float)(centerY + sq),   (float)(sq*3), (float)sq     },
        // Second 'b'
        { (float)(bx + sq*5 + sep),  (float)(centerY - sq*6), (float)sq,     (float)(sq*8) },
        { (float)(bx + sq*6 + sep),  (float)(centerY - sq*3), (float)(sq*4), (float)sq     },
        { (float)(bx + sq*9 + sep),  (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(bx + sq*6 + sep),  (float)(centerY + sq),   (float)(sq*3), (float)sq     },
        // 'l'
        { (float)(bx + sq*10 + sep*2), (float)(centerY - sq*6), (float)sq, (float)(sq*8) },
        // 'e'
        { (float)(bx + sq*11 + sep*3), (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(bx + sq*12 + sep*3), (float)(centerY - sq*3), (float)(sq*4), (float)sq     },
        { (float)(bx + sq*15 + sep*3), (float)(centerY - sq*3), (float)sq,     (float)(sq*3) },
        { (float)(bx + sq*12 + sep*3), (float)(centerY - sq),   (float)(sq*3), (float)sq     },
        { (float)(bx + sq*12 + sep*3), (float)(centerY + sq),   (float)(sq*4), (float)sq     },
        // 'r'
        { (float)(bx + sq*16 + sep*4), (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(bx + sq*17 + sep*4), (float)(centerY - sq*3), (float)(sq*4), (float)sq     },
        { (float)(bx + sq*20 + sep*4), (float)(centerY - sq*2), (float)sq,     (float)sq     },
    }, whiteColor);
}

// ─── Pixel-art "game over" logo ───────────────────────────────────────────────

void TextSystem::drawGameOverLogo(int centerX, int centerY, int sq, int sep,
                                  Color whiteColor, Color grayColor) {
    int totalWidth = (26 * sq) + (3 * sep);
    int startX = centerX - (totalWidth / 2);
    centerY = centerY - static_cast<int>(sq * 3.5f);

    // g
    drawRects({
        { (float)startX,         (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)startX,         (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*4),(float)(centerY - sq*2), (float)sq,     (float)(sq*10)},
        { (float)(startX + sq),  (float)(centerY + sq),   (float)(sq*3), (float)sq     },
        { (float)(startX + sq),  (float)(centerY + sq*7), (float)(sq*3), (float)sq     },
        { (float)startX,         (float)(centerY + sq*4), (float)sq,     (float)(sq*3) },
        { (float)(startX + sq),  (float)(centerY + sq*6), (float)sq,     (float)sq     },
    }, whiteColor);

    // a
    drawRects({
        { (float)(startX + sq*5 + sep), (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)(startX + sq*5 + sep), (float)(centerY - sq*2), (float)sq,     (float)(sq*3) },
        { (float)(startX + sq*9 + sep), (float)(centerY - sq*2), (float)sq,     (float)(sq*3) },
        { (float)(startX + sq*5 + sep), (float)(centerY + sq),   (float)(sq*7), (float)sq     },
    }, whiteColor);

    // m
    drawRects({
        { (float)(startX + sq*10 + sep*2), (float)(centerY - sq*3), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*11 + sep*2), (float)(centerY - sq*3), (float)(sq*2), (float)sq     },
        { (float)(startX + sq*12 + sep*2), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*13 + sep*2), (float)(centerY + sep),  (float)sq,     (float)sq     },
        { (float)(startX + sq*14 + sep*2), (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(startX + sq*15 + sep*2), (float)(centerY - sq*3), (float)(sq*2), (float)sq     },
        { (float)(startX + sq*16 + sep*2), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*17 + sep*2), (float)(centerY + sep),  (float)sq,     (float)sq     },
        { (float)(startX + sq*18 + sep*2), (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(startX + sq*19 + sep*2), (float)(centerY - sq*3), (float)(sq*2), (float)sq     },
        { (float)(startX + sq*20 + sep*2), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
    }, whiteColor);

    // e
    drawRects({
        { (float)(startX + sq*21 + sep*3), (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)(startX + sq*21 + sep*3), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*25 + sep*3), (float)(centerY - sq*2), (float)sq,     (float)(sq*2) },
        { (float)(startX + sq*22 + sep*3), (float)(centerY - sq),   (float)(sq*3), (float)sq     },
        { (float)(startX + sq*22 + sep*3), (float)(centerY + sq),   (float)(sq*4), (float)sq     },
    }, whiteColor);

    // "over" row
    centerY = centerY + sq*5 + sep;
    startX  = startX  + sq*5 + sep;

    // o
    drawRects({
        { (float)startX,         (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)startX,         (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*4),(float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)startX,         (float)(centerY + sq),   (float)(sq*4), (float)sq     },
    }, grayColor);

    // v
    drawRects({
        { (float)(startX + sq*5 + sep),  (float)(centerY - sq*3), (float)sq,     (float)(sq*5) },
        { (float)(startX + sq*6 + sep),  (float)(centerY + sq),   (float)(sq*3), (float)sq     },
        { (float)(startX + sq*8 + sep),  (float)centerY,          (float)sq,     (float)sq     },
        { (float)(startX + sq*9 + sep),  (float)(centerY - sq*3), (float)sq,     (float)(sq*4) },
    }, grayColor);

    // e (second)
    drawRects({
        { (float)(startX + sq*10 + sep*2), (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)(startX + sq*10 + sep*2), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*14 + sep*2), (float)(centerY - sq*2), (float)sq,     (float)(sq*2) },
        { (float)(startX + sq*11 + sep*2), (float)(centerY - sq),   (float)(sq*3), (float)sq     },
        { (float)(startX + sq*11 + sep*2), (float)(centerY + sq),   (float)(sq*4), (float)sq     },
    }, grayColor);

    // r
    drawRects({
        { (float)(startX + sq*15 + sep*3), (float)(centerY - sq*2), (float)sq,     (float)(sq*4) },
        { (float)(startX + sq*15 + sep*3), (float)(centerY - sq*3), (float)(sq*5), (float)sq     },
        { (float)(startX + sq*19 + sep*3), (float)(centerY - sq*2), (float)sq,     (float)sq     },
    }, grayColor);
}

// ─── Text draw ────────────────────────────────────────────────────────────────

void TextSystem::drawText(const std::string& text, int x, int y, int fs,
                          Color color, bool centered) {
    Vector2 size = MeasureTextEx(customFont, text.c_str(), static_cast<float>(fs), 1.0f);
    int drawX = centered ? x - static_cast<int>(size.x / 2) : x;
    int drawY = y - static_cast<int>(size.y / 2);
    DrawTextEx(customFont, text.c_str(),
               { static_cast<float>(drawX), static_cast<float>(drawY) },
               static_cast<float>(fs), 1.0f, color);
}
