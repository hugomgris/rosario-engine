#pragma once

#include <raylib.h>
#include "../../incs/DataStructs.hpp"

struct ParticleSpawnRequest {
    enum class ParticleType { Trail, Explosion, Dust, MenuTrail };
    ParticleType    type;
    float           x, y;
    int             count = 1;
    float           spawnInterval = 0.0f;
    unsigned int    emitterKey = 0u;
    Direction       direction;
    Color           color;
    bool            gridCoords = false; 
};