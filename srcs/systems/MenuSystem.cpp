#include "systems/MenuSystem.hpp"
#include "systems/TextSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include <raymath.h>
#include "RaylibColors.hpp"

// ─── Button ───────────────────────────────────────────────────────────────────

bool Button::isHovered(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, bounds);
}

void Button::render(bool hovered) const {
    Color bg      = hovered ? hoverColor      : backgroundColor;
    Color outline = hovered ? outlineHoverColor : outlineColor;
    Color text    = hovered ? textHoverColor  : textColor;

    DrawRectangleRec(bounds, bg);
    DrawRectangleLinesEx(bounds, 2.0f, outline);

    const char* cstr = this->text.c_str();
    int tw = MeasureText(cstr, 24);
    int tx = static_cast<int>(bounds.x + (bounds.width  - tw) / 2.0f);
    int ty = static_cast<int>(bounds.y + (bounds.height - 24) / 2.0f);
    DrawText(cstr, tx, ty, 24, text);
}

// ─── MenuSystem ───────────────────────────────────────────────────────────────

MenuSystem::MenuSystem() {}

void MenuSystem::init(int w, int h) {
    screenW = w;
    screenH = h;
    setState(GameState::Menu);
}

void MenuSystem::setState(GameState newState) {
    currentState = newState;
    buttons.clear();
    selectedIndex = -1;
    if (newState == GameState::Menu)
        buildStartButtons();
    else if (newState == GameState::GameOver)
        buildGameOverButtons();
}

// ─── Button builders ─────────────────────────────────────────────────────────

void MenuSystem::buildStartButtons() {
    const Color outline    = snakeALightSide;
    const Color bg         = customBlack;
    const Color text       = customWhite;
    const Color textHover  = customBlack;
    const Color hover      = customWhite;
    const Color outHover   = customWhite;

    float bw     = 300.0f;
    float bh     = 60.0f;
    float spacing = 20.0f;
    float startY  = screenH / 2.0f + 200.0f;
    float cx      = screenW / 2.0f - bw / 2.0f;

    // Start Game
    buttons.push_back({
        { cx, startY, bw, bh }, "Start Game",
        outline, bg, hover, text, textHover, outHover,
        [this]() { startRequested = true; }
    });

    // Change mode (placeholder — mode selection coming with config system)
    buttons.push_back({
        { cx, startY + bh + spacing, bw, bh }, "Change mode",
        outline, bg, hover, text, textHover, outHover,
        [this]() { /* TODO: cycle game mode */ }
    });

    // Quit
    buttons.push_back({
        { cx, startY + (bh + spacing) * 2.0f, bw, bh }, "Quit",
        outline, bg, hover, text, textHover, outHover,
        [this]() { quitRequested = true; }
    });
}

void MenuSystem::buildGameOverButtons() {
    const Color outline   = snakeALightSide;
    const Color bg        = customBlack;
    const Color text      = customWhite;
    const Color textHover = customBlack;
    const Color hover     = customWhite;
    const Color outHover  = customWhite;

    float bw      = 300.0f;
    float bh      = 60.0f;
    float spacing = 20.0f;
    float startY  = screenH / 2.0f + 200.0f;
    float cx      = screenW / 2.0f - bw / 2.0f;

    // Main Menu
    buttons.push_back({
        { cx, startY, bw, bh }, "Main Menu",
        outline, bg, hover, text, textHover, outHover,
        [this]() { menuRequested = true; }
    });

    // Quit
    buttons.push_back({
        { cx, startY + bh + spacing, bw, bh }, "Quit",
        outline, bg, hover, text, textHover, outHover,
        [this]() { quitRequested = true; }
    });
}

// ─── Update ───────────────────────────────────────────────────────────────────

void MenuSystem::update(float dt, ParticleSystem& particles) {
    spawnLogoTrail(particles);
    handleMouseInput();
    handleKeyboardInput();
    (void)dt;
}

void MenuSystem::spawnLogoTrail(ParticleSystem& particles) {
    ++trailCounter;
    if (trailCounter % 5 == 0) {
        float sq     = 30.0f;
        float trailX = screenW / 2.0f + sq * 18.2f;
        float trailY = screenH / 2.0f + sq * 3.2f;
        Color lightBlue = { 70, 130, 180, 255 };
        particles.spawnSnakeTrail(trailX, trailY, 1, 0.0f, lightBlue);
    }
}

void MenuSystem::handleMouseInput() {
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (auto& b : buttons) {
            if (b.isHovered(mousePos) && b.onClick)
                b.onClick();
        }
    }
}

void MenuSystem::handleKeyboardInput() {
    if (buttons.empty()) return;

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedIndex = (selectedIndex + 1) % static_cast<int>(buttons.size());
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedIndex = (selectedIndex - 1 + static_cast<int>(buttons.size()))
                        % static_cast<int>(buttons.size());
    }
    if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) && selectedIndex >= 0) {
        if (buttons[selectedIndex].onClick)
            buttons[selectedIndex].onClick();
    }
}

// ─── Render ───────────────────────────────────────────────────────────────────

void MenuSystem::renderMenu(TextSystem& textSystem, ParticleSystem& particles,
                            AnimationSystem* anim) {
    (void)particles;  // particles render themselves; we just draw UI on top

    if (anim) anim->renderTunnelEffect();

    int cx = screenW / 2;
    int cy = screenH / 2;
    int sq = 30, sep = 15;

    textSystem.drawLogo(cx, cy, sq, sep,
        Color{ 255, 248, 227, 255 },
        Color{  70, 130, 180, 255 },
        Color{ 254,  74,  81, 255 });

    // Buttons
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        bool hovered = buttons[i].isHovered(mousePos)
                    || i == selectedIndex;
        buttons[i].render(hovered);
    }

    // Fullscreen border
    DrawRectangleLinesEx({ 0, 0, (float)screenW, (float)screenH }, 25.0f, customWhite);
}

void MenuSystem::renderGameOver(TextSystem& textSystem, ParticleSystem& particles,
                                AnimationSystem* anim) {
    (void)particles;

    int cx = screenW / 2;
    int cy = screenH / 2;
    int sq = 30, sep = 15;

    textSystem.drawGameOverLogo(cx, cy, sq, sep,
        Color{ 255, 248, 227, 255 },
        Color{ 125, 125, 125, 255 });

    // Buttons
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        bool hovered = buttons[i].isHovered(mousePos)
                    || i == selectedIndex;
        buttons[i].render(hovered);
    }

    if (anim) anim->renderTunnelEffect();

    DrawRectangleLinesEx({ 0, 0, (float)screenW, (float)screenH }, 25.0f, customWhite);
}
