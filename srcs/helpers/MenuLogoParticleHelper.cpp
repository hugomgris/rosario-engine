#include "MenuLogoParticleHelper.hpp"
#include <limits>
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
#include "components/ParticleSpawnRequest.hpp"

bool MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(
	Registry& registry,
	Entity menuLogo,
	const ParticleConfig& particleConfig,
	std::vector<Entity>& emitters
) {
	if (!registry.hasComponent<PixelTextComponent>(menuLogo)
		|| !registry.hasComponent<PixelTextLayoutComponent>(menuLogo)) {
		return false;
	}

	const auto& text = registry.getComponent<PixelTextComponent>(menuLogo);
	const auto& layout = registry.getComponent<PixelTextLayoutComponent>(menuLogo);
	if (!text.visible || layout.quads.empty() || particleConfig.menuTrails.empty()) {
		return false;
	}

	// Calculate bounds from logo quads
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto& quad : layout.quads) {
		const Rectangle& r = quad.rect;
		minY = std::min(minY, r.y);
		maxX = std::max(maxX, r.x + r.width);
		maxY = std::max(maxY, r.y + r.height);
	}

	// Ensure emitter entities exist
	while (emitters.size() < particleConfig.menuTrails.size()) {
		emitters.push_back(registry.createEntity());
	}

	// Enqueue one spawn request per menu trail preset
	for (size_t i = 0; i < particleConfig.menuTrails.size(); ++i) {
		const auto& preset = particleConfig.menuTrails[i];
		ParticleSpawnRequest req;
		req.type = ParticleSpawnRequest::ParticleType::MenuTrail;
		req.spawnInterval = preset.spawnInterval;
		req.emitterKey = static_cast<unsigned int>(i + 1);
		if (preset.hasManualPosition) {
			req.x = preset.x;
			req.y = preset.y;
		} else {
			req.x = maxX;
			req.y = (minY + maxY) * 0.5f;
		}
		req.direction = preset.direction;
		req.color = preset.color;
		req.gridCoords = false;

		Entity emitter = emitters[i];
		if (registry.hasComponent<ParticleSpawnRequest>(emitter)) {
			registry.getComponent<ParticleSpawnRequest>(emitter) = req;
		} else {
			registry.addComponent<ParticleSpawnRequest>(emitter, req);
		}
	}

	// Clean up unused emitters
	for (size_t i = particleConfig.menuTrails.size(); i < emitters.size(); ++i) {
		if (registry.hasComponent<ParticleSpawnRequest>(emitters[i])) {
			registry.removeComponent<ParticleSpawnRequest>(emitters[i]);
		}
	}

	return true;
}
