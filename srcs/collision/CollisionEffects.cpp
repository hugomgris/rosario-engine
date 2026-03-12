#include "CollisionEffects.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/ScoreComponent.hpp"
#include "../components/FoodTag.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SolidTag.hpp"
#include "../components/ParticleSpawnRequest.hpp"
#include "../helpers/GameState.hpp"

namespace CollisionEffects {
    void GrowSnake(Registry& registry, Entity subject, Entity /*object*/, FrameContext& /*ctx*/) {
        if (!registry.hasComponent<SnakeComponent>(subject)) return;
        registry.getComponent<SnakeComponent>(subject).growing = true;
    }

    void RelocateFood(Registry& registry, Entity /*subject*/, Entity object, FrameContext& ctx) {
        if (!registry.hasComponent<FoodTag>(object)) return;

        // queue explosion at food's grid position
        // x/y are grid coords here — ParticleSystem converts via arenaBounds in ctx
        if (registry.hasComponent<PositionComponent>(object)) {
            const auto& pos = registry.getComponent<PositionComponent>(object).position;

            ParticleSpawnRequest req;
            req.type        = ParticleSpawnRequest::ParticleType::Explosion;
            req.x           = static_cast<float>(pos.x);
            req.y           = static_cast<float>(pos.y);
            req.color       = { 255, 100, 30, 255 };
            req.gridCoords  = true;
            registry.addComponent<ParticleSpawnRequest>(object, req);
        }

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