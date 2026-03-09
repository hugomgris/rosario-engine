#pragma once

// ─── GameState ────────────────────────────────────────────────────────────────
//
// Minimal state machine for the top-level game flow.
// The enum drives which systems are updated and rendered each frame.
// Transition logic lives in main.cpp to avoid coupling systems to each other.
//

enum class GameState {
    Menu,       // Title / start screen
    Playing,    // Active snake game
    GameOver,   // Results screen (transitions back to Menu)
};
