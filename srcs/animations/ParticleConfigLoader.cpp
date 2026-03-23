#include <fstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "ParticleConfigLoader.hpp"
#include "ParticleConfig.hpp"
#include "../../incs/third_party/json.hpp"

using json = nlohmann::json;

namespace {
	Direction parseDirection(const std::string& raw) {
		if (raw == "UP") return Direction::UP;
		if (raw == "DOWN") return Direction::DOWN;
		if (raw == "LEFT") return Direction::LEFT;
		if (raw == "RIGHT") return Direction::RIGHT;
		throw std::runtime_error("ParticleConfigLoader: invalid menuTrail.direction: " + raw);
	}

	Color parseColor(const json& node) {
		if (node.is_array()) {
			if (node.size() < 3 || node.size() > 4) {
				throw std::runtime_error("ParticleConfigLoader: menuTrail.color array must be [r,g,b] or [r,g,b,a]");
			}
			Color c;
			c.r = static_cast<unsigned char>(node.at(0).get<int>());
			c.g = static_cast<unsigned char>(node.at(1).get<int>());
			c.b = static_cast<unsigned char>(node.at(2).get<int>());
			c.a = static_cast<unsigned char>(node.size() == 4 ? node.at(3).get<int>() : 255);
			return c;
		}

		if (node.is_object()) {
			Color c;
			c.r = static_cast<unsigned char>(node.at("r").get<int>());
			c.g = static_cast<unsigned char>(node.at("g").get<int>());
			c.b = static_cast<unsigned char>(node.at("b").get<int>());
			c.a = static_cast<unsigned char>(node.value("a", 255));
			return c;
		}

		throw std::runtime_error("ParticleConfigLoader: menuTrail.color must be an array or object");
	}

	MenuTrailPreset parseMenuTrailPreset(const json& mt) {
		MenuTrailPreset preset;

		if (mt.contains("direction")) {
			preset.direction = parseDirection(mt.at("direction").get<std::string>());
		}
		if (mt.contains("color")) {
			preset.color = parseColor(mt.at("color"));
		}
		if (mt.contains("spawnInterval")) {
			preset.spawnInterval = mt.at("spawnInterval").get<float>();
		}

		const bool hasXKey = mt.contains("x");
		const bool hasYKey = mt.contains("y");
		const bool hasX = hasXKey && !mt.at("x").is_null();
		const bool hasY = hasYKey && !mt.at("y").is_null();

		if (hasX != hasY) {
			throw std::runtime_error("ParticleConfigLoader: menuTrail.x and menuTrail.y must be provided together");
		}

		if (hasX && hasY) {
			preset.x = mt.at("x").get<float>();
			preset.y = mt.at("y").get<float>();
			preset.hasManualPosition = true;
		}

		return preset;
	}
}

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

	config.menuTrails.clear();
	for (const auto& [key, value] : data.items()) {
		if (key == "menuTrail" || key.rfind("menuTrail_", 0) == 0) {
			if (!value.is_object()) {
				throw std::runtime_error("ParticleConfigLoader: " + key + " must be an object");
			}
			config.menuTrails.push_back(parseMenuTrailPreset(value));
		}
	}

	if (config.menuTrails.empty()) {
		config.menuTrails.push_back(MenuTrailPreset{});
	}

	return config;
}