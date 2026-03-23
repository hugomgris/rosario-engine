#pragma once

#include <vector>
#include "../incs/DataStructs.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "animations/ParticleConfig.hpp"

/**
 * Helper class for managing menu logo particle trails.
 * Handles enqueueing particle spawn requests for menu logo effects.
 */
class MenuLogoParticleHelper {
public:
	/**
	 * Enqueue menu logo trail particle requests based on configuration.
	 * Creates or updates particle spawn requests for each configured menu trail preset.
	 * Trails can emit from logo-derived positions or manual positions from config.
	 * @param registry The ECS registry
	 * @param menuLogo The menu logo entity (must have PixelTextComponent and PixelTextLayoutComponent)
	 * @param particleConfig The particle configuration containing menu trail presets
	 * @param emitters Output vector of emitter entities (will be resized as needed)
	 * @return true if requests were successfully enqueued, false if conditions not met
	 */
	static bool enqueueMenuLogoTrailRequests(
		Registry& registry,
		Entity menuLogo,
		const ParticleConfig& particleConfig,
		std::vector<Entity>& emitters
	);
};
