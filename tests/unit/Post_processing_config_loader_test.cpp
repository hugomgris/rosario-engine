#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "postprocessing/PostProcessConfigLoader.hpp"

// 1 - PostProcessingConfigLoader: Parse post-processing effect settings
TEST(PostProcessConfigLoader, ParsesConfigCorrectlyAndFully) {
	PostProcessConfigLoader::PresetTable presets;

	EXPECT_NO_THROW(presets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));

	EXPECT_EQ(presets.size(), 4);

	PostProcessConfig c1 = presets["crt_bloom"];
	EXPECT_EQ(c1.enabled, true);
	EXPECT_EQ(c1.effects[0], PostProcessEffect::CRT);
	EXPECT_EQ(c1.effects[1], PostProcessEffect::Bloom);
	EXPECT_EQ(c1.scanlineIntensity, 0.3f);
	EXPECT_EQ(c1.curvatureAmount, 0.04f);
	EXPECT_EQ(c1.vignetteStrength, 0.6f);
	EXPECT_EQ(c1.chromaticAberration, 0.0015f);
	EXPECT_EQ(c1.grainAmount, 0.08f);
	EXPECT_EQ(c1.bloomIntensity, 0.20f);

	PostProcessConfig c2 = presets["crt"];
	EXPECT_EQ(c2.enabled, true);
	EXPECT_EQ(c2.effects[0], PostProcessEffect::CRT);
	EXPECT_EQ(c2.scanlineIntensity, 0.15f);
	EXPECT_EQ(c2.curvatureAmount, 0.04f);
	EXPECT_EQ(c2.vignetteStrength, 0.6f);
	EXPECT_EQ(c2.chromaticAberration, 0.0015f);
	EXPECT_EQ(c2.grainAmount, 0.05f);
	EXPECT_EQ(c2.bloomIntensity, 0.0f);

	PostProcessConfig c3 = presets["menu"];
	EXPECT_EQ(c3.enabled, true);
	EXPECT_EQ(c3.effects[0], PostProcessEffect::Vignette);
	EXPECT_EQ(c3.scanlineIntensity, 0.0f);
	EXPECT_EQ(c3.curvatureAmount, 0.0f);
	EXPECT_EQ(c3.vignetteStrength, 0.2f);
	EXPECT_EQ(c3.chromaticAberration, 0.0f);
	EXPECT_EQ(c3.grainAmount, 0.0f);
	EXPECT_EQ(c3.bloomIntensity, 0.0f);

	PostProcessConfig c4 = presets["clean"];
	EXPECT_EQ(c4.enabled, false);
	EXPECT_EQ(c4.effects.size(), 0);
	EXPECT_EQ(c4.scanlineIntensity, 0.0f);
	EXPECT_EQ(c4.curvatureAmount, 0.0f);
	EXPECT_EQ(c4.vignetteStrength, 0.0f);
	EXPECT_EQ(c4.chromaticAberration, 0.0f);
	EXPECT_EQ(c4.grainAmount, 0.0f);
	EXPECT_EQ(c4.bloomIntensity, 0.0f);
}
