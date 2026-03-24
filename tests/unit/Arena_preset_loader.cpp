#include <gtest/gtest.h>

#include <cstdio>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>

#include "arena/ArenaPresetLoader.hpp"

namespace {
	std::string writeTempJson(const std::string& content) {
		const auto base = std::filesystem::temp_directory_path() /
			("rosario_ai_preset_test_" + std::to_string(std::rand()) + ".json");

		std::ofstream out(base);
		out << content;
		out.close();

		return base.string();
	}
}

// 1 - ArenaPresetLoader: Parse arena preset configurations
TEST(ArenaPresetLoader, ParseConfigurationCorrecly) {
	std::vector<WallPreset> presets = ArenaPresetLoader::load("data/ArenaPresets.json");

	EXPECT_EQ(presets.size(), 10);
	EXPECT_EQ(presets[0], WallPreset::InterLock1);
	EXPECT_EQ(presets[1], WallPreset::Spiral1);
	EXPECT_EQ(presets[2], WallPreset::Columns1);
	EXPECT_EQ(presets[3], WallPreset::Columns2);
	EXPECT_EQ(presets[4], WallPreset::Cross);
	EXPECT_EQ(presets[5], WallPreset::Checkerboard);
	EXPECT_EQ(presets[6], WallPreset::Maze);
	EXPECT_EQ(presets[7], WallPreset::Diamond);
	EXPECT_EQ(presets[8], WallPreset::Tunnels);
	EXPECT_EQ(presets[9], WallPreset::FourRooms);
}

// 2 ArenaPresetLoader: Wall matrix dimensions valid

