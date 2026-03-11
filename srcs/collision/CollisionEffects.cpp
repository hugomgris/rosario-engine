#include "CollisionEffects.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/ScoreComponent.hpp"
#include "../components/FoodTag.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SolidTag.hpp"
#include "../helpers/GameState.hpp"

namespace CollisionEffects {
    void GrowSnake(Registry& registry, Entity subject, Entity /*object*/, FrameContext& /*ctx*/) {
        if (!registry.hasComponent<SnakeComponent>(subject)) return;
        registry.getComponent<SnakeComponent>(subject).growing = true;
    }

    void RelocateFood(Registry& registry, Entity /*subject*/, Entity object, FrameContext& ctx) {
        if (!registry.hasComponent<FoodTag>(object)) return;
        GameState::relocateFood(registry, object, ctx.gridWidth, ctx.gridHeight, ctx.arena);
    }

    void IncrementScore(Registry& registry, Entity subject, Entity /*object*/, FrameContext& /*ctx*/) {
        if (!registry.hasComponent<ScoreComponent>(subject)) return;
        registry.getComponent<ScoreComponent>(subject).score += 1;
    }

    void KillSnake(Registry& registry, Entity subject, Entity /*object*/, FrameContext& ctx) {
        if (!registry.hasComponent<SnakeComponent>(subject)) return;
        ctx.playerDied = true;
    }

}