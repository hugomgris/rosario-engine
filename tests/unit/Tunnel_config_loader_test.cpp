#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "animations/TunnelConfigLoader.hpp"

// 1 - TunnelConfigLoader: Parse tunnel animation config
TEST(TunnelConfigLoader, ParsesConfigCorrectlyAndFully) {
	TunnelConfigLoader::PresetTable table;

	EXPECT_NO_THROW(table = TunnelConfigLoader::load("data/TunnelConfig.json"));

	EXPECT_EQ(table.size(), 2);

	TunnelConfig c1 = table["realm2D"];
	EXPECT_EQ(c1.borderThickness, 15);
	EXPECT_EQ(c1.contentInset, 80);
	EXPECT_EQ(c1.spawnInterval, 0.15f);
	EXPECT_EQ(c1.animationSpeed, 0.5f);
	EXPECT_EQ(c1.maxLines, 15);
	EXPECT_EQ(c1.lineColor.r, 70);
	EXPECT_EQ(c1.lineColor.g, 130);
	EXPECT_EQ(c1.lineColor.b, 180);
	EXPECT_EQ(c1.lineColor.a, 255);
	EXPECT_EQ(c1.lineThickness, 2.0f);
	
	TunnelConfig c2 = table["menu"];
	EXPECT_EQ(c2.borderThickness, 20);
	EXPECT_EQ(c2.contentInset, 80);
	EXPECT_EQ(c2.spawnInterval, 0.3f);
	EXPECT_EQ(c2.animationSpeed, 0.5f);
	EXPECT_EQ(c2.maxLines, 8);
	EXPECT_EQ(c2.lineColor.r, 70);
	EXPECT_EQ(c2.lineColor.g, 130);
	EXPECT_EQ(c2.lineColor.b, 180);
	EXPECT_EQ(c2.lineColor.a, 255);
	EXPECT_EQ(c2.lineThickness, 2.0f);
}

