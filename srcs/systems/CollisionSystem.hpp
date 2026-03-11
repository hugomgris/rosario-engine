#pragma once

#include "../ecs/Registry.hpp"
#include "../arena/ArenaGrid.hpp"
#include "../collision/CollisionRule.hpp"
#include "../collision/CollisionEffectDispatcher.hpp"
#include "../collision/CollisionEffects.hpp"
#include "../../incs/FrameContext.hpp"

class CollisionSystem {
private:
	void checkWallCollisions  (Registry& registry, const CollisionRuleTable& table, const CollisionEffectDispatcher& dispatcher, FrameContext& ctx);
	void checkSelfCollisions  (Registry& registry, const CollisionRuleTable& table, const CollisionEffectDispatcher& dispatcher, FrameContext& ctx);
	void checkSnakeCollisions (Registry& registry, const CollisionRuleTable& table, const CollisionEffectDispatcher& dispatcher, FrameContext& ctx);
	void checkFoodCollisions  (Registry& registry, const CollisionRuleTable& table, const CollisionEffectDispatcher& dispatcher, FrameContext& ctx);

	void resolveCollision(const std::string& subjectType,
						const std::string& objectType,
						Entity subject,
						Entity object,
						Registry& registry,
						const CollisionRuleTable& table,
						const CollisionEffectDispatcher& dispatcher,
						FrameContext& ctx);

public:
	void update(Registry& registry,
				const CollisionRuleTable& table,
				const CollisionEffectDispatcher& dispatcher,
				FrameContext& ctx);
};