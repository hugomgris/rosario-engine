#include <gtest/gtest.h>

#include "animations/ParticleConfig.hpp"

TEST(ConfigurationStructs, ParticleConfigSubConfigsAreAccessible) {
	ParticleConfig config;

	config.dustMaxDensity = 40;
	config.explosionCount = 25;
	config.trailScatter = 17.5f;

	EXPECT_EQ(config.dustMaxDensity, 40);
	EXPECT_EQ(config.explosionCount, 25);
	EXPECT_FLOAT_EQ(config.trailScatter, 17.5f);
}

TEST(ConfigurationStructs, ParticleConfigMenuTrailVectorOperationsWork) {
	ParticleConfig config;
	EXPECT_TRUE(config.menuTrails.empty());

	MenuTrailPreset trailA;
	trailA.direction = Direction::LEFT;

	MenuTrailPreset trailB;
	trailB.direction = Direction::DOWN;

	config.menuTrails.push_back(trailA);
	config.menuTrails.push_back(trailB);

	ASSERT_EQ(config.menuTrails.size(), 2);
	EXPECT_EQ(config.menuTrails[0].direction, Direction::LEFT);
	EXPECT_EQ(config.menuTrails[1].direction, Direction::DOWN);

	config.menuTrails.pop_back();
	ASSERT_EQ(config.menuTrails.size(), 1);
	EXPECT_EQ(config.menuTrails[0].direction, Direction::LEFT);
}

TEST(ConfigurationStructs, MenuTrailPresetColorAndDirectionAssignment) {
	MenuTrailPreset preset;
	preset.direction = Direction::UP;
	preset.color = Color{ 120, 80, 40, 200 };

	EXPECT_EQ(preset.direction, Direction::UP);
	EXPECT_EQ(preset.color.r, 120);
	EXPECT_EQ(preset.color.g, 80);
	EXPECT_EQ(preset.color.b, 40);
	EXPECT_EQ(preset.color.a, 200);
}

TEST(ConfigurationStructs, MenuTrailPresetManualPositionCanBeConfigured) {
	MenuTrailPreset preset;
	EXPECT_FALSE(preset.hasManualPosition);

	preset.hasManualPosition = true;
	preset.x = 320.5f;
	preset.y = 180.25f;

	EXPECT_TRUE(preset.hasManualPosition);
	EXPECT_FLOAT_EQ(preset.x, 320.5f);
	EXPECT_FLOAT_EQ(preset.y, 180.25f);
}

TEST(ConfigurationStructs, ParticleConfigDefaultValuesAreApplied) {
	ParticleConfig defaults;

	EXPECT_EQ(defaults.dustMaxDensity, 30);
	EXPECT_FLOAT_EQ(defaults.dustSpawnInterval, 0.15f);
	EXPECT_FLOAT_EQ(defaults.explosionMinSize, 1.0f);
	EXPECT_EQ(defaults.explosionCount, 20);
	EXPECT_FLOAT_EQ(defaults.trailScatter, 15.0f);
	EXPECT_FLOAT_EQ(defaults.trailSpawnInterval, 0.0f);
	EXPECT_TRUE(defaults.menuTrails.empty());
}
