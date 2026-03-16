#include "TextSystem.hpp"
#include <raylib.h>

TextSystem::~TextSystem() {
    if (_loaded)
        UnloadFont(_font);
}

void TextSystem::init() {
    loadFont();
    _loaded = true;
}

void TextSystem::loadFont() {
    // ASCII 32-255 + arrows + dot glyphs
    const int extra = 8;
    int count = (256 - 32) + extra;
    int* cp = new int[count];

    for (int i = 0; i < (256 - 32); ++i)
        cp[i] = 32 + i;

    cp[256 - 32 + 0] = 0x2191; // ↑
    cp[256 - 32 + 1] = 0x2193; // ↓
    cp[256 - 32 + 2] = 0x2190; // ←
    cp[256 - 32 + 3] = 0x2192; // →
    cp[256 - 32 + 4] = 0x00B7; // ·
    cp[256 - 32 + 5] = 0x2022; // •
    cp[256 - 32 + 6] = 0x00B7; // ·
    cp[256 - 32 + 7] = 0x002E; // .

    _font = LoadFontEx("fonts/JetBrainsMono-VariableFont_wght.ttf", 64, cp, count);
    delete[] cp;
}

Font& TextSystem::getFont() {
    return _font;
}

int TextSystem::measureText(const std::string& text, float fontSize) const {
    return static_cast<int>(MeasureTextEx(_font, text.c_str(), fontSize, 1.0f).x);
}

void TextSystem::drawText(const std::string& text, float x, float y,
                           float fontSize, Color color, bool centered) const {
    Vector2 size = MeasureTextEx(_font, text.c_str(), fontSize, 1.0f);
    float drawX  = centered ? x - size.x / 2.0f : x;
    float drawY  = y - size.y / 2.0f;
    DrawTextEx(_font, text.c_str(), { drawX, drawY }, fontSize, 1.0f, color);
}

// ---- Pixel-art rect helpers ----

void TextSystem::drawRects(const std::vector<Rectangle>& rects, Color color) const {
    for (const auto& r : rects)
        DrawRectangleRec(r, color);
}

void TextSystem::drawLogo(int centerX, int centerY, int square, int sep,
                           Color whiteColor, Color blueColor, Color redColor) const {
    int totalWidth = (26 * square) + (6 * sep);
    int startX = centerX - (totalWidth / 2);

    // n
    drawRects({
        { (float)startX,                          (float)(centerY - square * 3), (float)square,           (float)(square * 5) },
        { (float)(startX + square),               (float)(centerY - square * 3), (float)(square * 3),     (float)square       },
        { (float)(startX + square * 3),           (float)(centerY - square * 2), (float)(square * 2),     (float)square       },
        { (float)(startX + square * 4),           (float)(centerY - square),     (float)square,           (float)(square * 3) },
    }, whiteColor);

    // i base
    drawRects({
        { (float)(startX + square * 5 + sep),     (float)(centerY - square * 4), (float)square,           (float)(square * 7) },
        { (float)(startX + square * 5 + sep),     (float)(centerY + square * 3), (float)(square * 27),    (float)square       },
    }, blueColor);

    // i dot
    drawRects({
        { (float)(startX + square * 5 + sep),     (float)(centerY - square * 6), (float)square,           (float)square       },
    }, redColor);

    // bbler
    int bx = startX + square * 6 + sep * 2;
    drawRects({
        // b
        { (float)bx,                              (float)(centerY - square * 6), (float)square,           (float)(square * 8) },
        { (float)(bx + square),                   (float)(centerY - square * 3), (float)(square * 4),     (float)square       },
        { (float)(bx + square * 4),               (float)(centerY - square * 2), (float)square,           (float)(square * 4) },
        { (float)(bx + square),                   (float)(centerY + square),     (float)(square * 3),     (float)square       },
        // b
        { (float)(bx + square * 5 + sep),         (float)(centerY - square * 6), (float)square,           (float)(square * 8) },
        { (float)(bx + square * 6 + sep),         (float)(centerY - square * 3), (float)(square * 4),     (float)square       },
        { (float)(bx + square * 9 + sep),         (float)(centerY - square * 2), (float)square,           (float)(square * 4) },
        { (float)(bx + square * 6 + sep),         (float)(centerY + square),     (float)(square * 3),     (float)square       },
        // l
        { (float)(bx + square * 10 + sep * 2),    (float)(centerY - square * 6), (float)square,           (float)(square * 8) },
        // e
        { (float)(bx + square * 11 + sep * 3),    (float)(centerY - square * 3), (float)square,           (float)(square * 5) },
        { (float)(bx + square * 12 + sep * 3),    (float)(centerY - square * 3), (float)(square * 4),     (float)square       },
        { (float)(bx + square * 15 + sep * 3),    (float)(centerY - square * 3), (float)square,           (float)(square * 3) },
        { (float)(bx + square * 12 + sep * 3),    (float)(centerY - square),     (float)(square * 3),     (float)square       },
        { (float)(bx + square * 12 + sep * 3),    (float)(centerY + square),     (float)(square * 4),     (float)square       },
        // r
        { (float)(bx + square * 16 + sep * 4),    (float)(centerY - square * 3), (float)square,           (float)(square * 5) },
        { (float)(bx + square * 17 + sep * 4),    (float)(centerY - square * 3), (float)(square * 4),     (float)square       },
        { (float)(bx + square * 20 + sep * 4),    (float)(centerY - square * 2), (float)square,           (float)square       },
    }, whiteColor);
}

void TextSystem::drawGameOverLogo(int centerX, int centerY, int square, int sep,
                                   Color whiteColor, Color grayColor) const {
    int totalWidth = (26 * square) + (3 * sep);
    int startX = centerX - (totalWidth / 2);
    centerY = centerY - (int)(square * 3.5f);

    // g
    drawRects({
        { (float)startX,              (float)(centerY - square * 3), (float)(square * 5), (float)square        },
        { (float)startX,              (float)(centerY - square * 2), (float)square,       (float)(square * 4)  },
        { (float)(startX + square*4), (float)(centerY - square * 2), (float)square,       (float)(square * 10) },
        { (float)(startX + square),   (float)(centerY + square),     (float)(square * 3), (float)square        },
        { (float)(startX + square),   (float)(centerY + square * 7), (float)(square * 3), (float)square        },
        { (float)startX,              (float)(centerY + square * 4), (float)square,       (float)(square * 3)  },
        { (float)(startX + square),   (float)(centerY + square * 6), (float)square,       (float)square        },
    }, whiteColor);

    // a
    drawRects({
        { (float)(startX + square*5 + sep),  (float)(centerY - square*3), (float)(square*5), (float)square       },
        { (float)(startX + square*5 + sep),  (float)(centerY - square*2), (float)square,     (float)(square*3)   },
        { (float)(startX + square*9 + sep),  (float)(centerY - square*2), (float)square,     (float)(square*3)   },
        { (float)(startX + square*5 + sep),  (float)(centerY + square),   (float)(square*7), (float)square       },
    }, whiteColor);

    // m
    drawRects({
        { (float)(startX + square*10 + sep*2), (float)(centerY - square*3), (float)square,     (float)(square*4) },
        { (float)(startX + square*11 + sep*2), (float)(centerY - square*3), (float)(square*2), (float)square     },
        { (float)(startX + square*12 + sep*2), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*13 + sep*2), (float)(centerY),            (float)square,     (float)square     },
        { (float)(startX + square*14 + sep*2), (float)(centerY - square*3), (float)square,     (float)(square*5) },
        { (float)(startX + square*15 + sep*2), (float)(centerY - square*3), (float)(square*2), (float)square     },
        { (float)(startX + square*16 + sep*2), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*17 + sep*2), (float)(centerY),            (float)square,     (float)square     },
        { (float)(startX + square*18 + sep*2), (float)(centerY - square*3), (float)square,     (float)(square*5) },
        { (float)(startX + square*19 + sep*2), (float)(centerY - square*3), (float)(square*2), (float)square     },
        { (float)(startX + square*20 + sep*2), (float)(centerY - square*2), (float)square,     (float)(square*4) },
    }, whiteColor);

    // e
    drawRects({
        { (float)(startX + square*21 + sep*3), (float)(centerY - square*3), (float)(square*5), (float)square     },
        { (float)(startX + square*21 + sep*3), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*25 + sep*3), (float)(centerY - square*2), (float)square,     (float)(square*2) },
        { (float)(startX + square*22 + sep*3), (float)(centerY - square),   (float)(square*3), (float)square     },
        { (float)(startX + square*22 + sep*3), (float)(centerY + square),   (float)(square*4), (float)square     },
    }, whiteColor);

    // "over" row — shifted down and inset
    centerY = centerY + square * 5 + sep;
    startX  = startX  + square * 5 + sep;

    // o
    drawRects({
        { (float)startX,              (float)(centerY - square*3), (float)(square*5), (float)square     },
        { (float)startX,              (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*4), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)startX,              (float)(centerY + square),   (float)(square*4), (float)square     },
    }, grayColor);

    // v
    drawRects({
        { (float)(startX + square*5 + sep),  (float)(centerY - square*3), (float)square,     (float)(square*5) },
        { (float)(startX + square*6 + sep),  (float)(centerY + square),   (float)(square*3), (float)square     },
        { (float)(startX + square*8 + sep),  (float)(centerY),            (float)square,     (float)square     },
        { (float)(startX + square*9 + sep),  (float)(centerY - square*3), (float)square,     (float)(square*4) },
    }, grayColor);

    // e
    drawRects({
        { (float)(startX + square*10 + sep*2), (float)(centerY - square*3), (float)(square*5), (float)square     },
        { (float)(startX + square*10 + sep*2), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*14 + sep*2), (float)(centerY - square*2), (float)square,     (float)(square*2) },
        { (float)(startX + square*11 + sep*2), (float)(centerY - square),   (float)(square*3), (float)square     },
        { (float)(startX + square*11 + sep*2), (float)(centerY + square),   (float)(square*4), (float)square     },
    }, grayColor);

    // r
    drawRects({
        { (float)(startX + square*15 + sep*3), (float)(centerY - square*2), (float)square,     (float)(square*4) },
        { (float)(startX + square*15 + sep*3), (float)(centerY - square*3), (float)(square*5), (float)square     },
        { (float)(startX + square*19 + sep*3), (float)(centerY - square*2), (float)square,     (float)square     },
    }, grayColor);
}
