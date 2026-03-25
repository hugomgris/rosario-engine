#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "AI/AIPresetLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "arena/ArenaPresetLoader.hpp"

namespace {
	std::string writeTempJson(const std::string& content) {
		const auto base = std::filesystem::temp_directory_path() /
			("rosario_all_loaders_test_" + std::to_string(std::rand()) + ".json");

		std::ofstream out(base);
		out << content;
		out.close();

		return base.string();
	}
}

// 1 - All Loaders: Missing config file throws appropriate exception
TEST(AllLoaders, MissingConfigFileThrows) {
	EXPECT_THROW(AIPresetLoader::load("data/missing.json"), std::runtime_error);
	EXPECT_THROW(TunnelConfigLoader::load("data/missing.json"), std::runtime_error);
	EXPECT_THROW(PostProcessConfigLoader::load("data/missing.json"), std::runtime_error);
	EXPECT_THROW(ParticleConfigLoader::load("data/missing.json"), std::runtime_error);
	EXPECT_THROW(CollisionRuleLoader::load("data/missing.json"), std::runtime_error);
	EXPECT_THROW(ArenaPresetLoader::load("data/missing.json"), std::runtime_error);
}

// 2 - All Loaders: Corrupted JSON throws parsing exception
TEST(AllLoaders, CorruptedJsonThrowsParsingException) {
	const std::string path = writeTempJson(R"JSON(
	{
		"broken": [1, 2,
	)JSON");

	auto expectParseRuntimeError = [](const auto& fn) {
		try {
			fn();
			FAIL() << "Expected std::runtime_error due to parse failure";
		} catch (const std::runtime_error& e) {
			EXPECT_NE(std::string(e.what()).find("parse"), std::string::npos);
		} catch (...) {
			FAIL() << "Expected std::runtime_error";
		}
	};

	expectParseRuntimeError([&] { (void)AIPresetLoader::load(path); });
	expectParseRuntimeError([&] { (void)TunnelConfigLoader::load(path); });
	expectParseRuntimeError([&] { (void)PostProcessConfigLoader::load(path); });
	expectParseRuntimeError([&] { (void)ParticleConfigLoader::load(path); });
	expectParseRuntimeError([&] { (void)CollisionRuleLoader::load(path); });
	expectParseRuntimeError([&] { (void)ArenaPresetLoader::load(path); });

	std::remove(path.c_str());
}
