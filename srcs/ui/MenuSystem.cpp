#include "MenuSystem.hpp"
#include "../systems/ParticleSystem.hpp"
#include "../../incs/RaylibColors.hpp"

// ---- Construction / state ----

MenuSystem::MenuSystem(int screenWidth, int screenHeight)
    : _screenWidth(screenWidth), _screenHeight(screenHeight) {}

void MenuSystem::setState(MenuState state, GameMode currentMode) {
    _menuState     = state;
    _selectedIndex = -1;
    initButtons(currentMode);
}

// ---- Button initialisation ----

void MenuSystem::initButtons(GameMode currentMode) {
    _buttons.clear();

    const Color outline    = snakeALightSide;
    const Color bg         = customBlack;
    const Color textCol    = customWhite;
    const Color hoverBg    = customWhite;
    const Color hoverText  = customBlack;
    const Color hoverOutline = customWhite;

    const float w       = 300.0f;
    const float h       = 60.0f;
    const float spacing = 20.0f;
    const float startY  = _screenHeight / 2.0f + 200.0f;
    const float centerX = _screenWidth  / 2.0f - w / 2.0f;

    auto makeButton = [&](const std::string& label, float y) -> Button {
        Button b;
        b.bounds           = { centerX, y, w, h };
        b.label            = label;
        b.outlineColor     = outline;
        b.backgroundColor  = bg;
        b.textColor        = textCol;
        b.hoverColor       = hoverBg;
        b.textHoverColor   = hoverText;
        b.outlineHoverColor = hoverOutline;
        return b;
    };

    if (_menuState == MenuState::Start) {
        Button start = makeButton("Start Game",   startY);
        Button mode  = makeButton("Change mode",  startY + h + spacing);
        Button quit  = makeButton("Quit",         startY + (h + spacing) * 2);

        // actions are empty — update() maps button index to MenuAction
        _buttons = { start, mode, quit };

    } else { // GameOver
        Button menu = makeButton("Main Menu", startY);
        Button quit = makeButton("Quit",      startY + h + spacing);
        _buttons = { menu, quit };
    }
    (void)currentMode; // reserved: could colour the mode button differently
}

void MenuSystem::clearButtons() {
    _buttons.clear();
}

// ---- Navigation helpers ----

void MenuSystem::selectNext() {
    if (_buttons.empty()) return;
    if (_selectedIndex < 0)
        _selectedIndex = 0;
    else
        _selectedIndex = (_selectedIndex + 1) % static_cast<int>(_buttons.size());
}

void MenuSystem::selectPrev() {
    if (_buttons.empty()) return;
    if (_selectedIndex < 0)
        _selectedIndex = static_cast<int>(_buttons.size()) - 1;
    else {
        _selectedIndex--;
        if (_selectedIndex < 0)
            _selectedIndex = static_cast<int>(_buttons.size()) - 1;
    }
}

MenuSystem::NavAction MenuSystem::pollNavigation() const {
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) return NavAction::Up;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) return NavAction::Down;
    if (IsKeyPressed(KEY_ENTER))                        return NavAction::Confirm;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)) return NavAction::Cancel;
    return NavAction::None;
}

// ---- Update ----

MenuAction MenuSystem::update(float dt, GameMode currentMode, ParticleSystem& particles) {
    (void)dt;

    // Logo snake trail particle — only in Start state
    if (_menuState == MenuState::Start) {
        _logoTrailCounter++;
        if (_logoTrailCounter % 2 == 0) {
            const int   square = 30;
            const float tx     = _screenWidth  / 2.0f + square * 18.2f;
            const float ty     = _screenHeight / 2.0f + square *  3.2f;
            particles.spawnMenuTrailAt(tx, ty, { 70, 130, 180, 255 });
        }
    }

    // Keyboard navigation
    NavAction nav = pollNavigation();
    switch (nav) {
        case NavAction::Up:      selectPrev(); break;
        case NavAction::Down:    selectNext(); break;
        case NavAction::Cancel:
            return (_menuState == MenuState::Start) ? MenuAction::Quit : MenuAction::None;
        case NavAction::Confirm:
            if (_selectedIndex >= 0 && _selectedIndex < static_cast<int>(_buttons.size()))
                goto handle_confirm;
            break;
        default: break;
    }

    // Mouse hover / click
    {
        Vector2 mouse = GetMousePosition();
        for (int i = 0; i < static_cast<int>(_buttons.size()); ++i) {
            if (_buttons[i].isHovered(mouse)) {
                _selectedIndex = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    goto handle_confirm;
            }
        }
    }

    return MenuAction::None;

handle_confirm:
    if (_menuState == MenuState::Start) {
        switch (_selectedIndex) {
            case 0: return MenuAction::StartGame;
            case 1: return MenuAction::SwitchMode;
            case 2: return MenuAction::Quit;
        }
    } else { // GameOver
        switch (_selectedIndex) {
            case 0: return MenuAction::Restart;
            case 1: return MenuAction::Quit;
        }
    }
    return MenuAction::None;
}

// ---- Render ----

static const char* modeName(GameMode m) {
    switch (m) {
        case GameMode::SINGLE: return "Mode: SINGLE PLAYER";
        case GameMode::MULTI:  return "Mode: MULTIPLAYER";
        case GameMode::VSAI:   return "Mode: VS AI";
    }
    return "";
}

void MenuSystem::render(TextSystem& text, GameMode currentMode) const {
    // Fullscreen border
    const float bThick = 25.0f;
    DrawRectangleLinesEx({ 0.0f, 0.0f, (float)_screenWidth, (float)_screenHeight },
                         bThick, Color{ 147, 112, 219, 255 });

    const int   cx     = _screenWidth  / 2;
    const int   cy     = _screenHeight / 2;
    const int   square = 30;
    const int   sep    = 15;

    text.drawLogo(cx, cy, square, sep,
                  Color{ 255, 248, 227, 255 },
                  Color{  70, 130, 180, 255 },
                  Color{ 254,  74,  81, 255 });

    // Mode label
    const char* mLabel = modeName(currentMode);
    int mw = text.measureText(mLabel, 28.0f);
    text.drawText(mLabel,
                  static_cast<float>(cx - mw / 2),
                  static_cast<float>(cy + 135 + 14), // +14 = half font height approx
                  28.0f, Color{ 255, 248, 227, 255 });

    // Buttons
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < static_cast<int>(_buttons.size()); ++i) {
        bool hovered = _buttons[i].isHovered(mouse) || (i == _selectedIndex);
        _buttons[i].render(const_cast<Font&>(const_cast<TextSystem&>(text).getFont()), hovered);
    }
}

void MenuSystem::renderGameOver(TextSystem& text, GameMode currentMode) const {
    // Fullscreen border
    const float bThick = 25.0f;
    DrawRectangleLinesEx({ 0.0f, 0.0f, (float)_screenWidth, (float)_screenHeight },
                         bThick, Color{ 147, 112, 219, 255 });

    const int cx     = _screenWidth  / 2;
    const int cy     = _screenHeight / 2;
    const int square = 30;
    const int sep    = 15;

    text.drawGameOverLogo(cx, cy, square, sep,
                          Color{ 255, 248, 227, 255 },
                          Color{ 125, 125, 125, 255 });

    // Buttons
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < static_cast<int>(_buttons.size()); ++i) {
        bool hovered = _buttons[i].isHovered(mouse) || (i == _selectedIndex);
        _buttons[i].render(const_cast<Font&>(const_cast<TextSystem&>(text).getFont()), hovered);
    }
    (void)currentMode;
}
