#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../../incs/FrameContext.hpp"

namespace CollisionEffects {
    using EffectFn = void(*)(Registry&, Entity subject, Entity object, FrameContext&);

    void GrowSnake      (Registry&, Entity subject, Entity object, FrameContext&);
    void RelocateFood   (Registry&, Entity subject, Entity object, FrameContext&);
    void IncrementScore (Registry&, Entity subject, Entity object, FrameContext&);
    void KillSnake      (Registry&, Entity subject, Entity object, FrameContext&);
}