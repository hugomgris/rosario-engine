#pragma once
#include <vector>
#include "DataStructs.hpp"

struct AIComponent {
    // Runtime state
    std::vector<Vec2> path;
    Vec2              target  = { 0, 0 };

    // Behaviour config (mirrors old AIConfig)
    AIBehaviourState behavior          = AIBehaviourState::MEDIUM;
    int              maxSearchDepth    = 150;
    bool             useSafetyCheck    = true;
    bool             hasSurvivalMode   = true;
    bool             predictOpponent   = false;
    float            randomMoveChance  = 0.0f;
    float            aggressiveness    = 0.5f;

    // Preset factories
    static AIComponent easy() {
        return { {}, {0,0}, AIBehaviourState::EASY,  50,  false, false, false, 0.15f, 0.8f };
    }
    static AIComponent medium() {
        return { {}, {0,0}, AIBehaviourState::MEDIUM, 150, true,  true,  false, 0.0f,  0.5f };
    }
    static AIComponent hard() {
        return { {}, {0,0}, AIBehaviourState::HARD,   300, true,  true,  true,  0.0f,  0.3f };
    }
};