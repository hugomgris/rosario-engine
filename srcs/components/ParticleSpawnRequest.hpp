#pragma once

#include <raylib.h>
#include "../../incs/DataStructs.hpp"

struct ParticleSpawnRequest {
    enum class ParticleType { Trail, Explosion, Dust, MenuTrail };
    ParticleType    type;
    float           x, y;
    int             count;
    Direction       direction;
    Color           color;
    bool            gridCoords = false; 
};