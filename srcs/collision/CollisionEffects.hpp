#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../arena/ArenaGrid.hpp"

namespace CollisionEffects {
    struct EffectContext {
        const ArenaGrid*    arena;
        int                 gridWidth;
        int                 gridHeight;
        bool*               playerDied;
    };

    using EffectFn = void(*)(Registry&, Entity subject, Entity object, const EffectContext&);

    void GrowSnake      (Registry&, Entity subject, Entity object, const EffectContext&);
    void RelocateFood   (Registry&, Entity subject, Entity object, const EffectContext&);
    void IncrementScore (Registry&, Entity subject, Entity object, const EffectContext&);
    void KillSnake      (Registry&, Entity subject, Entity object, const EffectContext&);
}