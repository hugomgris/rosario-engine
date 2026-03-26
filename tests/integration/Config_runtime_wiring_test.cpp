#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "AI/AIPresetLoader.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "animations/TunnelConfigLoader.hpp"
#include "arena/ArenaPresetLoader.hpp"
#include "collision/CollisionRuleLoader.hpp"
#include "components/ParticleSpawnRequest.hpp"
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
#include "ecs/Registry.hpp"
#include "helpers/MenuLogoParticleHelper.hpp"
#include "postprocessing/PostProcessConfigLoader.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "ui/ButtonConfigLoader.hpp"
#include "ui/GlyphLibraryLoader.hpp"
#include "ui/GlyphPresetLoader.hpp"
#include "ui/PixelTextHelper.hpp"
#include "ui/PixelTextLayoutSystem.hpp"

namespace {
std::string writeTempJson(const std::string& prefix, const std::string& content) {
	const auto path = std::filesystem::temp_directory_path() /
		(prefix + "_" + std::to_string(std::rand()) + ".json");

	std::ofstream out(path);
	out << content;
	out.close();

	return path.string();
}

std::string makeGlyphPresetJson(const std::string& gameOverText, bool includeGameOverPreset) {
	if (includeGameOverPreset) {
		return R"JSON(
{
	"presets": [
		{
			"id": "gameover_title",
			"text": ")JSON" + gameOverText + R"JSON(",
			"x": 640,
			"y": 180,
			"scale": 2.0,
			"centerX": true,
			"visible": false,
			"color": [255, 255, 255, 255],
			"visibleInStates": ["GameOver"]
		},
		{
			"id": "menu_logo",
			"text": "ROSARIO",
			"x": 640,
			"y": 120,
			"scale": 2.0,
			"centerX": true,
			"visible": false,
			"color": [255, 255, 255, 255],
			"visibleInStates": ["Menu"]
		}
	]
}
)JSON";
	}

	return R"JSON(
{
	"presets": [
		{
			"id": "menu_logo",
			"text": "ROSARIO",
			"x": 640,
			"y": 120,
			"scale": 2.0,
			"centerX": true,
			"visible": false,
			"color": [255, 255, 255, 255],
			"visibleInStates": ["Menu"]
		}
	]
}
)JSON";
}

std::string makeParticleConfigJson(const std::string& direction,
							   int r,
							   int g,
							   int b,
							   float spawnInterval,
							   int x,
							   int y) {
	return R"JSON(
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
	"menuTrail_primary": {
		"direction": ")JSON" + direction + R"JSON(",
		"color": [)JSON" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + R"JSON(, 255],
		"spawnInterval": )JSON" + std::to_string(spawnInterval) + R"JSON(,
		"x": )JSON" + std::to_string(x) + R"JSON(,
		"y": )JSON" + std::to_string(y) + R"JSON(
	}
}
)JSON";
}
}

TEST(ConfigRuntimeWiring, BootLoadsAllRuntimeJsonAndBindsWithoutCrossInterference) {
	CollisionRuleTable rules;
	AIPresetLoader::PresetTable aiPresets;
	ParticleConfig particleConfig;
	PostProcessConfigLoader::PresetTable postProcessPresets;
	TunnelConfigLoader::PresetTable tunnelPresets;
	ArenaPresetLoader::PresetList arenaPresets;
	GlyphLibrary glyphLibrary;
	GlyphPresetLoader::PresetTable glyphPresets;
	ButtonConfigLoader::ButtonTable buttonTable;

	EXPECT_NO_THROW(rules = CollisionRuleLoader::load("data/CollisionRules.json"));
	EXPECT_NO_THROW(aiPresets = AIPresetLoader::load("data/AIPresets.json"));
	EXPECT_NO_THROW(particleConfig = ParticleConfigLoader::load("data/ParticleConfig.json"));
	EXPECT_NO_THROW(postProcessPresets = PostProcessConfigLoader::load("data/PostProcessConfig.json"));
	EXPECT_NO_THROW(tunnelPresets = TunnelConfigLoader::load("data/TunnelConfig.json"));
	EXPECT_NO_THROW(arenaPresets = ArenaPresetLoader::load("data/ArenaPresets.json"));
	EXPECT_NO_THROW(glyphLibrary = GlyphLibraryLoader::load("data/GlyphLibrary.json"));
	EXPECT_NO_THROW(glyphPresets = GlyphPresetLoader::load("data/GlyphPresets.json"));
	EXPECT_NO_THROW(buttonTable = ButtonConfigLoader::load("data/ButtonConfig.json"));

	ASSERT_NE(rules.find("Snake", "Food"), nullptr);
	ASSERT_TRUE(aiPresets.find("easy") != aiPresets.end());
	ASSERT_FALSE(particleConfig.menuTrails.empty());
	ASSERT_TRUE(postProcessPresets.find("crt_bloom") != postProcessPresets.end());
	ASSERT_TRUE(tunnelPresets.find("realm2D") != tunnelPresets.end());
	ASSERT_FALSE(arenaPresets.empty());
	ASSERT_FALSE(glyphLibrary.glyphsById.empty());
	ASSERT_TRUE(glyphPresets.find("gameover_title") != glyphPresets.end());
	ASSERT_FALSE(buttonTable.start.empty());

	ParticleSystem particleSystem(1920, 1080, particleConfig);
	AnimationSystem animationSystem;
	animationSystem.enable(true, tunnelPresets.at("realm2D"));

	PixelTextComponent gameOverTemplate = PixelTextHelper::makeGameOverTitleTemplate(glyphPresets);
	EXPECT_EQ(gameOverTemplate.id, "gameover_title");
	EXPECT_FALSE(gameOverTemplate.text.empty());

	EXPECT_EQ(particleSystem.getParticleCount(), 0u);
	EXPECT_EQ(aiPresets.at("easy").behavior, AIBehaviourState::EASY);
	EXPECT_GT(tunnelPresets.at("realm2D").maxLines, 0);
}

TEST(ConfigRuntimeWiring, HotReloadGlyphPresetsUpdatesVisibleUiTextEntitiesWithoutRestart) {
	const std::string pathV1 = writeTempJson("glyph_reload_v1", makeGlyphPresetJson("GAME OVER V1", true));
	const std::string pathV2 = writeTempJson("glyph_reload_v2", makeGlyphPresetJson("GAME OVER V2", true));

	GlyphPresetLoader::PresetTable presetsV1;
	GlyphPresetLoader::PresetTable presetsV2;
	ASSERT_NO_THROW(presetsV1 = GlyphPresetLoader::load(pathV1));
	ASSERT_NO_THROW(presetsV2 = GlyphPresetLoader::load(pathV2));

	Registry registry;
	Entity textEntity = registry.createEntity();

	registry.addComponent<PixelTextComponent>(textEntity, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(textEntity, PixelTextLayoutComponent{});

	PixelTextComponent templateV1 = PixelTextHelper::makeGameOverTitleTemplate(presetsV1);
	PixelTextHelper::applyPixelTextTemplate(registry, textEntity, templateV1);
	auto& textV1 = registry.getComponent<PixelTextComponent>(textEntity);
	EXPECT_EQ(textV1.text, "GAME OVER V1");

	PixelTextComponent templateV2 = PixelTextHelper::makeGameOverTitleTemplate(presetsV2);
	PixelTextHelper::applyPixelTextTemplate(registry, textEntity, templateV2);

	auto& text = registry.getComponent<PixelTextComponent>(textEntity);
	auto& layout = registry.getComponent<PixelTextLayoutComponent>(textEntity);
	EXPECT_EQ(text.text, "GAME OVER V2");
	EXPECT_EQ(text.id, "gameover_title");
	EXPECT_TRUE(layout.dirty);

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::GameOver);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(textEntity).visible);

	std::remove(pathV1.c_str());
	std::remove(pathV2.c_str());
}

TEST(ConfigRuntimeWiring, HotReloadFallbackWorksWhenGameOverPresetRemoved) {
	const std::string pathNoGameOver = writeTempJson("glyph_reload_no_gameover", makeGlyphPresetJson("IGNORED", false));

	GlyphPresetLoader::PresetTable presets;
	ASSERT_NO_THROW(presets = GlyphPresetLoader::load(pathNoGameOver));

	PixelTextComponent fallbackTemplate = PixelTextHelper::makeGameOverTitleTemplate(presets);
	EXPECT_EQ(fallbackTemplate.id, "gameover_title");
	EXPECT_EQ(fallbackTemplate.text, "GAME OVER");
	EXPECT_FLOAT_EQ(fallbackTemplate.scale, 2.0f);
	EXPECT_FALSE(fallbackTemplate.visible);
	ASSERT_EQ(fallbackTemplate.visibleInStates.size(), 1u);
	EXPECT_EQ(fallbackTemplate.visibleInStates[0], GameState::GameOver);

	Registry registry;
	Entity fallbackEntity = registry.createEntity();
	registry.addComponent<PixelTextComponent>(fallbackEntity, fallbackTemplate);
	registry.addComponent<PixelTextLayoutComponent>(fallbackEntity, PixelTextLayoutComponent{});

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::GameOver);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(fallbackEntity).visible);

	GlyphLibrary glyphLibrary;
	ASSERT_NO_THROW(glyphLibrary = GlyphLibraryLoader::load("data/GlyphLibrary.json"));

	PixelTextLayoutSystem layoutSystem;
	layoutSystem.update(registry, glyphLibrary);

	const auto& layout = registry.getComponent<PixelTextLayoutComponent>(fallbackEntity);
	EXPECT_FALSE(layout.quads.empty());

	std::remove(pathNoGameOver.c_str());
}

TEST(ConfigRuntimeWiring, ReloadingParticleConfigUpdatesMenuTrailRequestOnNextFrame) {
	const std::string cfgV1Path = writeTempJson(
		"particle_reload_v1",
		makeParticleConfigJson("LEFT", 255, 165, 0, 0.10f, 120, 220)
	);
	const std::string cfgV2Path = writeTempJson(
		"particle_reload_v2",
		makeParticleConfigJson("UP", 70, 130, 180, 0.25f, 400, 500)
	);

	ParticleConfig cfgV1;
	ParticleConfig cfgV2;
	ASSERT_NO_THROW(cfgV1 = ParticleConfigLoader::load(cfgV1Path));
	ASSERT_NO_THROW(cfgV2 = ParticleConfigLoader::load(cfgV2Path));

	Registry registry;
	Entity menuLogo = registry.createEntity();

	PixelTextComponent logoText;
	logoText.id = "menu_logo";
	logoText.text = "ROSARIO";
	logoText.visible = true;
	registry.addComponent<PixelTextComponent>(menuLogo, logoText);

	PixelTextLayoutComponent logoLayout;
	logoLayout.quads.push_back(PixelTextQuad{ Rectangle{100.0f, 200.0f, 40.0f, 20.0f}, WHITE });
	registry.addComponent<PixelTextLayoutComponent>(menuLogo, logoLayout);

	std::vector<Entity> emitters;
	ASSERT_TRUE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, menuLogo, cfgV1, emitters));
	ASSERT_EQ(emitters.size(), 1u);
	ASSERT_TRUE(registry.hasComponent<ParticleSpawnRequest>(emitters[0]));

	const auto& reqV1 = registry.getComponent<ParticleSpawnRequest>(emitters[0]);
	EXPECT_EQ(reqV1.direction, Direction::LEFT);
	EXPECT_FLOAT_EQ(reqV1.spawnInterval, 0.10f);
	EXPECT_EQ(reqV1.color.r, 255);
	EXPECT_EQ(reqV1.color.g, 165);
	EXPECT_EQ(reqV1.color.b, 0);
	EXPECT_FLOAT_EQ(reqV1.x, 120.0f);
	EXPECT_FLOAT_EQ(reqV1.y, 220.0f);

	ASSERT_TRUE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, menuLogo, cfgV2, emitters));
	ASSERT_EQ(emitters.size(), 1u);

	const auto& reqV2 = registry.getComponent<ParticleSpawnRequest>(emitters[0]);
	EXPECT_EQ(reqV2.direction, Direction::UP);
	EXPECT_FLOAT_EQ(reqV2.spawnInterval, 0.25f);
	EXPECT_EQ(reqV2.color.r, 70);
	EXPECT_EQ(reqV2.color.g, 130);
	EXPECT_EQ(reqV2.color.b, 180);
	EXPECT_FLOAT_EQ(reqV2.x, 400.0f);
	EXPECT_FLOAT_EQ(reqV2.y, 500.0f);

	std::remove(cfgV1Path.c_str());
	std::remove(cfgV2Path.c_str());
}
