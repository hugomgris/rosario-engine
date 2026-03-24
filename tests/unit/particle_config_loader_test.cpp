#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "animations/ParticleConfigLoader.hpp"

namespace {
std::string writeTempJson(const std::string& content) {
	const auto base = std::filesystem::temp_directory_path() /
		("rosario_particle_config_test_" + std::to_string(std::rand()) + ".json");

	std::ofstream out(base);
	out << content;
	out.close();

	return base.string();
}

const MenuTrailPreset* findPresetByColor(const ParticleConfig& config, unsigned char r, unsigned char g, unsigned char b) {
	for (const auto& preset : config.menuTrails) {
		if (preset.color.r == r && preset.color.g == g && preset.color.b == b) {
			return &preset;
		}
	}
	return nullptr;
}
}

// 1 - ParticleConfigLoader: All sections parse correctly (dust, explosion, trail, menuTrail*)
TEST(ParticleConfigLoader, ParsesAllSectionsCorrectly) {
	const std::string path = writeTempJson(R"JSON(
		{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		}
	}
	)JSON");

	ParticleConfig config;
	EXPECT_NO_THROW(config = ParticleConfigLoader::load(path));

	EXPECT_EQ(config.dustMaxDensity, 20);
	EXPECT_EQ(config.dustSpawnInterval, 0.08f);
	EXPECT_EQ(config.dustMinSize, 3.0f);
	EXPECT_EQ(config.dustMaxSize, 15.0f);
	EXPECT_EQ(config.dustMinLifetime, 3.0f);
	EXPECT_EQ(config.dustMaxLifetime, 6.0f);
	EXPECT_NE(config.dustMinSize, 183.09123f);
	
	EXPECT_EQ(config.explosionMinSize, 4.0f);
	EXPECT_EQ(config.explosionMaxSize, 18.0f);
	EXPECT_EQ(config.explosionMinLifetime, 0.3f);
	EXPECT_EQ(config.explosionMaxLifetime, 0.9f);
	EXPECT_EQ(config.explosionMinSpeed, 80.0f);
	EXPECT_EQ(config.explosionMaxSpeed, 350.0f);
	EXPECT_EQ(config.explosionCount, 28);
	EXPECT_NE(config.explosionCount, 92139);

	EXPECT_EQ(config.trailMinSize, 10.0f);
	EXPECT_EQ(config.trailMaxSize, 20.0f);
	EXPECT_EQ(config.trailMinLifetime, 0.2f);
	EXPECT_EQ(config.trailMaxLifetime, 1.0f);
	EXPECT_EQ(config.trailMinSpeed, 50.0f);
	EXPECT_EQ(config.trailMaxSpeed, 60.0f);
	EXPECT_EQ(config.trailCount, 3);
	EXPECT_EQ(config.trailScatter, 15.0f);
	EXPECT_EQ(config.trailSpawnInterval, 0.05f);
	EXPECT_NE(config.trailMinLifetime, 12.5f);

	std::remove(path.c_str());
}

// 2 - ParticleConfigLoader: Multiple menuTrail entries
TEST(ParticleConfigLoader, ParsesMultipleMenuTrailSets) {
	const std::string path = writeTempJson(R"JSON(
	{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		},
		"menuTrail_yellow": {
			"direction": "LEFT",
			"color": [255, 165, 0, 255],
			"spawnInterval": 0.1,
			"x": 160,
			"y": 700
		},
		"menuTrail_blue": {
			"direction": "UP",
			"color": {"r": 70, "g": 130, "b": 180, "a": 255},
			"spawnInterval": 0.2,
			"x": 160,
			"y": 750
		}
	}
	)JSON");

	ParticleConfig config;
	ASSERT_NO_THROW(config = ParticleConfigLoader::load(path));

	EXPECT_EQ(config.menuTrails.size(), 2u);

	const MenuTrailPreset* yellow = findPresetByColor(config, 255, 165, 0);
	ASSERT_NE(yellow, nullptr);
	EXPECT_EQ(yellow->direction, Direction::LEFT);
	EXPECT_FLOAT_EQ(yellow->spawnInterval, 0.1f);
	EXPECT_TRUE(yellow->hasManualPosition);
	EXPECT_FLOAT_EQ(yellow->x, 160.0f);
	EXPECT_FLOAT_EQ(yellow->y, 700.0f);

	const MenuTrailPreset* blue = findPresetByColor(config, 70, 130, 180);
	ASSERT_NE(blue, nullptr);
	EXPECT_EQ(blue->direction, Direction::UP);
	EXPECT_FLOAT_EQ(blue->spawnInterval, 0.2f);
	EXPECT_TRUE(blue->hasManualPosition);
	EXPECT_FLOAT_EQ(blue->x, 160.0f);
	EXPECT_FLOAT_EQ(blue->y, 750.0f);

	std::remove(path.c_str());
}

// 3 - ParticleConfigLoader: color parsing
TEST(ParticleConfigLoader, ParsesColorsCorrectly) {
	const std::string path = writeTempJson(R"JSON(
	{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		},
		"menuTrail_yellow": {
			"direction": "LEFT",
			"color": [255, 165, 0, 255],
			"spawnInterval": 0.1,
			"x": 160,
			"y": 700
		},
		"menuTrail_blue": {
			"direction": "UP",
			"color": {"r": 70, "g": 130, "b": 180, "a": 255},
			"spawnInterval": 0.2,
			"x": 160,
			"y": 750
		}
	}
	)JSON");

	ParticleConfig config;
	ASSERT_NO_THROW(config = ParticleConfigLoader::load(path));

	EXPECT_EQ(config.menuTrails.size(), 2u);

	const MenuTrailPreset* yellow = findPresetByColor(config, 255, 165, 0);
	ASSERT_NE(yellow, nullptr);
	EXPECT_EQ(yellow->direction, Direction::LEFT);
	EXPECT_FLOAT_EQ(yellow->spawnInterval, 0.1f);
	EXPECT_TRUE(yellow->hasManualPosition);
	EXPECT_FLOAT_EQ(yellow->x, 160.0f);
	EXPECT_FLOAT_EQ(yellow->y, 700.0f);

	const MenuTrailPreset* blue = findPresetByColor(config, 70, 130, 180);
	ASSERT_NE(blue, nullptr);
	EXPECT_EQ(blue->direction, Direction::UP);
	EXPECT_FLOAT_EQ(blue->spawnInterval, 0.2f);
	EXPECT_TRUE(blue->hasManualPosition);
	EXPECT_FLOAT_EQ(blue->x, 160.0f);
	EXPECT_FLOAT_EQ(blue->y, 750.0f);

	std::remove(path.c_str());
}

// 4 - ParticleConfigLoader: Direction enum parsing (UP/DOWN/LEFT/RIGHT)
TEST(ParticleConfigLoader, DirectionEnumParsing) {
	const std::string path = writeTempJson(R"JSON(
	{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		},
		"menuTrail_one": {
			"direction": "LEFT",
			"color": [1, 2, 3, 255],
			"spawnInterval": 0.1,
			"x": 160,
			"y": 700
		},
		"menuTrail_two": {
			"direction": "UP",
			"color": {"r": 4, "g": 5, "b": 6, "a": 255},
			"spawnInterval": 0.2,
			"x": 160,
			"y": 750
		},
		"menuTrail_three": {
			"direction": "RIGHT",
			"color": {"r": 7, "g": 8, "b": 9, "a": 255},
			"spawnInterval": 0.2,
			"x": 160,
			"y": 750
		},
		"menuTrail_four": {
			"direction": "DOWN",
			"color": {"r": 10, "g": 11, "b": 12, "a": 255},
			"spawnInterval": 0.2,
			"x": 160,
			"y": 750
		}
	}
	)JSON");

	ParticleConfig config;
	ASSERT_NO_THROW(config = ParticleConfigLoader::load(path));

	EXPECT_EQ(config.menuTrails.size(), 4u);

	const MenuTrailPreset* trail_one = findPresetByColor(config, 1, 2, 3);
	EXPECT_EQ(trail_one->direction, Direction::LEFT);

	const MenuTrailPreset* trail_two = findPresetByColor(config, 4, 5, 6);
	EXPECT_EQ(trail_two->direction, Direction::UP);

	const MenuTrailPreset* trail_three = findPresetByColor(config, 7, 8, 9);
	EXPECT_EQ(trail_three->direction, Direction::RIGHT);

	const MenuTrailPreset* trail_four = findPresetByColor(config, 10, 11, 12);
	EXPECT_EQ(trail_four->direction, Direction::DOWN);

	std::remove(path.c_str());
}

// 5 - ParticleConfigLoader: Incomplete values throw exception
TEST(ParticleConfigLoader, RejectsHalfManualPosition) {
	const std::string path = writeTempJson(R"JSON(
	{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		},
		"menuTrail_broken": {
			"direction": "LEFT",
			"color": [135, 0, 147, 255],
			"spawnInterval": 0.1,
			"x": 160
		}
	}
	)JSON");

	ParticleConfig config;
	EXPECT_ANY_THROW(config = ParticleConfigLoader::load(path));

	std::remove(path.c_str());
}

// 6 - ParticleConfigLoader: Out-of-range values throw exception
TEST(ParticleConfigLoader, RejectsOutOfRangeValues) {
	const std::string path = writeTempJson(R"JSON(
	{
		"dust": {
			"maxDensity": 20,
			"spawnInterval": 0.08,
			"minSize": 3.0,
			"maxSize": 15.0,
			"minLifetime": 3.0,
			"maxLifetime": 6.0
		},
		"explosion": {
			"minSize": 4.0,
			"maxSize": 18.0,
			"minLifetime": 0.3,
			"maxLifetime": 0.9,
			"minSpeed": 80.0,
			"maxSpeed": 350.0,
			"count": 28
		},
		"trail": {
			"minSize": 10.0,
			"maxSize": 20.0,
			"minLifetime": 0.2,
			"maxLifetime": 1.0,
			"minSpeed": 50.0,
			"maxSpeed": 60.0,
			"count": 3,
			"scatter": 15.0,
			"spawnInterval": 0.05
		},
		"menuTrail_one": {
			"direction": "BAD",
			"color": [1, 2, 3, 255],
			"spawnInterval": 0.1,
			"x": 160,
			"y": 700
		}
	}
	)JSON");

	ParticleConfig config;
	ASSERT_ANY_THROW(config = ParticleConfigLoader::load(path));
}
