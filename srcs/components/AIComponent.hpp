#pragma once

#include <vector>
#include "../../incs/DataStructs.hpp"

struct AIComponent {
    std::vector<Vec2>   path;
    Vec2                target = { 0, 0 };

    AIBehaviourState    behavior            = AIBehaviourState::MEDIUM;
    int                 maxSearchDepth      = 150;
    bool                useSafetyCheck      = true;
    bool                hasSurvivalMode     = true;
    bool                predictOpponent     = false;
    float               randomMoveChance    = 0.0f;
    float               aggresiveness       = 0.5f;
};