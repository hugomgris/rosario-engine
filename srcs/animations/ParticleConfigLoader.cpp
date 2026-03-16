#include <fstream>
#include <stdexcept>
#include "ParticleConfigLoader.hpp"
#include "ParticleConfig.hpp"
#include "../../incs/third_party/json.hpp"

using json = nlohmann::json;

ParticleConfig ParticleConfigLoader::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("ParticleConfigLoader: cannot open: " + path);

	json data;
	try {
		file >> data;
	} catch (const json::parse_error& e) {
		throw std::runtime_error("ParticleConfigLoader: parse error: " + std::string(e.what()));
	}

	ParticleConfig config;

	const auto& dust = data.at("dust");
	config.dustMaxDensity       = dust.at("maxDensity").get<int>();
	config.dustSpawnInterval    = dust.at("spawnInterval").get<float>();
	config.dustMinSize          = dust.at("minSize").get<float>();
	config.dustMaxSize          = dust.at("maxSize").get<float>();
	config.dustMinLifetime      = dust.at("minLifetime").get<float>();
	config.dustMaxLifetime      = dust.at("maxLifetime").get<float>();

	const auto& explosion = data.at("explosion");
	config.explosionMinSize         = explosion.at("minSize").get<float>();
	config.explosionMaxSize         = explosion.at("maxSize").get<float>();
	config.explosionMinLifetime     = explosion.at("minLifetime").get<float>();
	config.explosionMaxLifetime     = explosion.at("maxLifetime").get<float>();
	config.explosionMinSpeed        = explosion.at("minSpeed").get<float>();
	config.explosionMaxSpeed        = explosion.at("maxSpeed").get<float>();
	config.explosionCount           = explosion.at("count").get<int>();

	const auto& trail = data.at("trail");
	config.trailMinSize         = trail.at("minSize").get<float>();
	config.trailMaxSize         = trail.at("maxSize").get<float>();
	config.trailMinLifetime     = trail.at("minLifetime").get<float>();
	config.trailMaxLifetime     = trail.at("maxLifetime").get<float>();
	config.trailMinSpeed        = trail.at("minSpeed").get<float>();
	config.trailMaxSpeed        = trail.at("maxSpeed").get<float>();
	config.trailCount           = trail.at("count").get<int>();
	config.trailScatter         = trail.at("scatter").get<float>();
	if (trail.contains("spawnInterval"))
		config.trailSpawnInterval = trail.at("spawnInterval").get<float>();

	if (data.contains("menuTrail")) {
		const auto& mt = data.at("menuTrail");
		config.menuTrailMinSize     = mt.at("minSize").get<float>();
		config.menuTrailMaxSize     = mt.at("maxSize").get<float>();
		config.menuTrailMinLifetime = mt.at("minLifetime").get<float>();
		config.menuTrailMaxLifetime = mt.at("maxLifetime").get<float>();
		config.menuTrailMinSpeed    = mt.at("minSpeed").get<float>();
		config.menuTrailMaxSpeed    = mt.at("maxSpeed").get<float>();
		config.menuTrailCount       = mt.at("count").get<int>();
		config.menuTrailScatter     = mt.at("scatter").get<float>();
	}

	return config;
}