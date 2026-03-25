#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "collision/CollisionRuleLoader.hpp"

namespace {
	std::string writeTempJson(const std::string& content) {
		const auto base = std::filesystem::temp_directory_path() /
			("rosario_collision_rule_test_" + std::to_string(std::rand()) + ".json");

		std::ofstream out(base);
		out << content;
		out.close();

		return base.string();
	}
}

// 1 - CollisionRuleLoader: Parse collision rules from JSON
// 2 - CollisionRuleLoader: Subject/object pairs correctly stored
// 2 for 1 because 1 implies 2
TEST(CollisionRuleLoader, ParsesRulesCorrectlyAndFully) {
	CollisionRuleTable rules;

	EXPECT_NO_THROW(rules = CollisionRuleLoader::load("data/CollisionRules.json"));

	EXPECT_EQ(rules.rules.size(), 4);

	EXPECT_EQ(rules.rules[0].subject, "Snake");
	EXPECT_EQ(rules.rules[0].object, "Food");
	EXPECT_EQ(rules.rules[0].effects.size(), 3);
	EXPECT_EQ(rules.rules[0].effects[0], "GrowSnake");
	EXPECT_EQ(rules.rules[0].effects[1], "RelocateFood");
	EXPECT_EQ(rules.rules[0].effects[2], "IncrementScore");

	EXPECT_EQ(rules.rules[1].subject, "Snake");
	EXPECT_EQ(rules.rules[1].object, "Wall");
	EXPECT_EQ(rules.rules[1].effects.size(), 1);
	EXPECT_EQ(rules.rules[1].effects[0], "KillSnake");
	

	EXPECT_EQ(rules.rules[2].subject, "Snake");
	EXPECT_EQ(rules.rules[2].object, "Self");
	EXPECT_EQ(rules.rules[2].effects.size(), 1);
	EXPECT_EQ(rules.rules[2].effects[0], "KillSnake");
}

// 3 - CollisionRuleLoader: Malformed rules throw exception
TEST(CollisionRuleLoader, MalformedRulesThrow) {
	const std::string path = writeTempJson(R"JSON(
	{
		"collisionRules": [
			{
				"subject": "Snake",
				"object": "Food",
				"effects": ["GrowSnake", "RelocateFood", "IncrementScore"]
			},
			{
				"subject": "BAD",
				"object": "WRONG",
			}
		]
	}
	)JSON");

	CollisionRuleTable rules;

	EXPECT_ANY_THROW(rules = CollisionRuleLoader::load(path));
	EXPECT_THROW(rules = CollisionRuleLoader::load(path), std::runtime_error);

	std::remove(path.c_str());
}