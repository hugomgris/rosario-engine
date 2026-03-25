#include <gtest/gtest.h>

#include <vector>
#include <string>

#include "arena/ArenaPresetLoader.hpp"

// 1 - ArenaPresetLoader: Parse arena preset configurations
TEST(ArenaPresetLoader, ParseConfigurationCorrecly) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));

	EXPECT_EQ(presets.size(), 10);
	EXPECT_EQ(presets[0].name, "InterLock1");
	EXPECT_EQ(presets[1].name, "Spiral1");
	EXPECT_EQ(presets[2].name, "Columns1");
	EXPECT_EQ(presets[3].name, "Columns2");
	EXPECT_EQ(presets[4].name, "Cross");
	EXPECT_EQ(presets[5].name, "Checkerboard");
	EXPECT_EQ(presets[6].name, "Maze");
	EXPECT_EQ(presets[7].name, "Diamond");
	EXPECT_EQ(presets[8].name, "Tunnels");
	EXPECT_EQ(presets[9].name, "FourRooms");
}

// 2 ArenaPresetLoader: Wall matrix dimensions valid
TEST(ArenaPresetLoader, WallMatrixDimensionsValid) {
	ArenaPresetLoader::PresetList presets;
	EXPECT_NO_THROW(presets = ArenaPresetLoader::load("data/ArenaPresets.json"));

	for (const auto& preset : presets) {
		EXPECT_EQ(preset.height, 32) << "Preset: " << preset.name;
		EXPECT_EQ(preset.width, 32) << "Preset: " << preset.name;
		EXPECT_EQ(static_cast<int>(preset.walls.size()), preset.height) << "Preset: " << preset.name;

		for (const auto& row : preset.walls) {
			EXPECT_EQ(static_cast<int>(row.size()), preset.width) << "Preset: " << preset.name;
		}
	}
}

