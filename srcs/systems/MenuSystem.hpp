#pragma once
#include <string>
#include <vector>
#include <functional>
#include <raylib.h>
#include "core/GameState.hpp"
#include "systems/ParticleSystem.hpp"

// ─── Button ───────────────────────────────────────────────────────────────────

struct Button {
    Rectangle  bounds           = {};
    std::string text;
    Color outlineColor          = WHITE;
    Color backgroundColor       = BLACK;
    Color hoverColor            = WHITE;
    Color textColor             = WHITE;
    Color textHoverColor        = BLACK;
    Color outlineHoverColor     = WHITE;
    std::function<void()> onClick;

    bool  isHovered(Vector2 mousePos) const;
    void  render(bool hovered) const;
};

// ─── MenuSystem ───────────────────────────────────────────────────────────────
//
// Handles all non-gameplay screens:  Start menu  and  Game Over  screen.
// It does NOT own any ECS state — the caller (main.cpp) passes the needed
// context (current GameState, ParticleSystem) via update()/render() arguments.
//
// Button onClick lambdas communicate intent back to the caller through
// the transitionTo / quitRequested flags rather than direct calls.
//
class MenuSystem {
public:
    MenuSystem();
    void init(int screenW = 1920, int screenH = 1080);

    // Called once when the visible screen changes
    void setState(GameState newState);

    // Per-frame calls
    void update(float dt, ParticleSystem& particles);

    void renderMenu    (class TextSystem& text, ParticleSystem& particles,
                        class AnimationSystem* anim = nullptr);
    void renderGameOver(class TextSystem& text, ParticleSystem& particles,
                        class AnimationSystem* anim = nullptr);

    // ── State transitions requested by buttons ────────────────────────────
    bool wantsStart()  const { return startRequested;  }
    bool wantsMenu()   const { return menuRequested;   }
    bool wantsQuit()   const { return quitRequested;   }
    void consumeFlags()      { startRequested = menuRequested = quitRequested = false; }

private:
    GameState       currentState     = GameState::Menu;
    std::vector<Button> buttons;
    int             selectedIndex    = -1;
    int             screenW          = 1920;
    int             screenH          = 1080;

    // Particle helpers
    float           trailTimer        = 0.0f;
    int             trailCounter      = 0;

    // Transition flags set by button lambdas
    bool startRequested  = false;
    bool menuRequested   = false;
    bool quitRequested   = false;

    void buildStartButtons();
    void buildGameOverButtons();
    void spawnLogoTrail(ParticleSystem& particles);

    // Input
    void handleMouseInput();
    void handleKeyboardInput();
};
