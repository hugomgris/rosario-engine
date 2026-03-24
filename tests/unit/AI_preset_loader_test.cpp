#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "AI/AIPresetLoader.hpp"

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

// 1 - AIPresetLoader: Parse AI presets with difficulty levels
// 2 - AIPresetLoader: AIPreset data structures match config
// 2x1 because passing the first test makes an implicit assertion that the second one passes too
TEST(AIPresetLoader, ParseAllDifficultyLevels) {
	AIPresetLoader::PresetTable preset = AIPresetLoader::load("data/AIPresets.json");

	EXPECT_EQ(preset.size(), 3);

	EXPECT_EQ(preset["easy"].behavior, AIBehaviourState::EASY);
	EXPECT_EQ(preset["medium"].behavior, AIBehaviourState::MEDIUM);
	EXPECT_EQ(preset["hard"].behavior, AIBehaviourState::HARD);
	EXPECT_NE(preset["easy"].behavior, AIBehaviourState::HARD);

	EXPECT_EQ(preset["easy"].maxSearchDepth, 50);
	EXPECT_TRUE(preset["easy"].useSafetyCheck == false);
	EXPECT_TRUE(preset["easy"].hasSurvivalMode == false);
	EXPECT_EQ(preset["easy"].randomMoveChance, 0.15f);
	EXPECT_EQ(preset["easy"].aggresiveness, 0.8f);

	EXPECT_EQ(preset["medium"].maxSearchDepth, 150);
	EXPECT_TRUE(preset["medium"].useSafetyCheck == true);
	EXPECT_TRUE(preset["medium"].hasSurvivalMode == true);
	EXPECT_EQ(preset["medium"].randomMoveChance, 0.0f);
	EXPECT_EQ(preset["medium"].aggresiveness, 0.5f);
	
	EXPECT_EQ(preset["hard"].maxSearchDepth, 300);
	EXPECT_TRUE(preset["hard"].useSafetyCheck == true);
	EXPECT_TRUE(preset["hard"].hasSurvivalMode == true);
	EXPECT_EQ(preset["hard"].randomMoveChance, 0.0f);
	EXPECT_EQ(preset["hard"].aggresiveness, 0.3f);
}

// 3 - AIPresetLoader: Invalid difficulty level throws exception
TEST(AIPresetLoader, InvalidDifficultyThrows) {
	const std::string path = writeTempJson(R"JSON(
	{
		"AIPresets": [
        {
            "name":             "superfluous",
            "behavior":         "SUPERFLUOUS",
            "maxSearchDepth":   50,
            "useSafetyCheck":   false,
            "hasSurvivalMode":  false,
            "randomMoveChance": 0.15,
            "aggressiveness":   0.8
        }
    ]
	}
	)JSON");

	AIPresetLoader::PresetTable preset;
	EXPECT_ANY_THROW(AIPresetLoader::load(path));
	EXPECT_THROW(AIPresetLoader::load(path), std::runtime_error);
}

