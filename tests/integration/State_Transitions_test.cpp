#include <gtest/gtest.h>

#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "components/SnakeComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/ScoreComponent.hpp"
#include "components/FoodTag.hpp"
#include "components/SolidTag.hpp"
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
#include "components/ButtonComponent.hpp"
#include "ui/PixelTextHelper.hpp"
#include "ui/MenuSystem.hpp"
#include "ui/ButtonConfigLoader.hpp"
#include "ui/GlyphLibraryLoader.hpp"
#include "ui/GlyphPresetLoader.hpp"
#include "helpers/GameManager.hpp"
#include "systems/InputSystem.hpp"
#include "AI/AIPresetLoader.hpp"
#include "FrameContext.hpp"

namespace {

struct GameEntityCounts {
	int snakes = 0;
	int food = 0;
	int solids = 0;
	int texts = 0;
	int buttons = 0;
};

GameEntityCounts countGameplayEntities(Registry& registry) {
	GameEntityCounts counts;

	for (auto entity : registry.view<SnakeComponent>()) {
		(void)entity;
		counts.snakes++;
	}

	for (auto entity : registry.view<FoodTag>()) {
		(void)entity;
		counts.food++;
	}

	for (auto entity : registry.view<SolidTag>()) {
		(void)entity;
		counts.solids++;
	}

	for (auto entity : registry.view<PixelTextComponent>()) {
		(void)entity;
		counts.texts++;
	}

	for (auto entity : registry.view<ButtonComponent>()) {
		(void)entity;
		counts.buttons++;
	}

	return counts;
}

}

// 1 - Menu -> Playing transition resets world and spawns expected gameplay entities
TEST(StateTransitions, MenuToPlayingResetsWorldAndSpawnsEntities) {
	Registry registry;
	InputSystem inputSystem;
	ArenaGrid arena(32, 32);
	AIPresetLoader::PresetTable aiPresets;
	ASSERT_NO_THROW(aiPresets = AIPresetLoader::load("data/AIPresets.json"));

	Entity playerSnake(0u), secondSnake(0u), food(0u);
	GameMode mode = GameMode::SINGLE;

	GameEntityCounts beforeTransition = countGameplayEntities(registry);
	EXPECT_EQ(beforeTransition.snakes, 0);
	EXPECT_EQ(beforeTransition.food, 0);

	ASSERT_NO_THROW(GameManager::resetGame(
		registry,
		inputSystem,
		playerSnake,
		secondSnake,
		food,
		32,
		32,
		arena,
		aiPresets,
		mode
	));

	GameEntityCounts afterTransition = countGameplayEntities(registry);
	EXPECT_EQ(afterTransition.snakes, 1);
	EXPECT_EQ(afterTransition.food, 1);
	EXPECT_TRUE(registry.hasComponent<PositionComponent>(food));
}

// 2 - Playing -> GameOver transition swaps UI layout and state-visible text correctly
TEST(StateTransitions, PlayingToGameOverSwapsUILayout) {
	Registry registry;

	GlyphLibrary glyphLibrary;
	ASSERT_NO_THROW(glyphLibrary = GlyphLibraryLoader::load("data/GlyphLibrary.json"));
	GlyphPresetLoader::PresetTable glyphPresets;
	ASSERT_NO_THROW(glyphPresets = GlyphPresetLoader::load("data/GlyphPresets.json"));

	Entity playingText = registry.createEntity();
	registry.addComponent<PixelTextComponent>(playingText, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(playingText, PixelTextLayoutComponent{});
	registry.getComponent<PixelTextComponent>(playingText).text = "Score: 100";
	registry.getComponent<PixelTextComponent>(playingText).visibleInStates = {GameState::Playing};
	registry.getComponent<PixelTextComponent>(playingText).visible = false;

	Entity gameOverText = registry.createEntity();
	registry.addComponent<PixelTextComponent>(gameOverText, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(gameOverText, PixelTextLayoutComponent{});
	registry.getComponent<PixelTextComponent>(gameOverText).text = "YOUR FINAL SCORE: 100";
	registry.getComponent<PixelTextComponent>(gameOverText).visibleInStates = {GameState::GameOver};
	registry.getComponent<PixelTextComponent>(gameOverText).visible = false;

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::Playing);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(playingText).visible);
	EXPECT_FALSE(registry.getComponent<PixelTextComponent>(gameOverText).visible);

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::GameOver);
	EXPECT_FALSE(registry.getComponent<PixelTextComponent>(playingText).visible);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(gameOverText).visible);
}

// 3 - ReturnToMenu action rehydrates world and menu entities cleanly (no accumulation)
TEST(StateTransitions, ReturnToMenuRehydratesWorldAndMenuWithoutAccumulation) {
	Registry registry;
	InputSystem inputSystem;
	ArenaGrid arena(32, 32);
	AIPresetLoader::PresetTable aiPresets;
	ASSERT_NO_THROW(aiPresets = AIPresetLoader::load("data/AIPresets.json"));
	ButtonConfigLoader::ButtonTable menuButtons;
	ASSERT_NO_THROW(menuButtons = ButtonConfigLoader::load("data/ButtonConfig.json"));
	MenuSystem menuSystem(1920, 1080);

	Entity playerSnake(0u), secondSnake(0u), food(0u);
	GameMode mode = GameMode::SINGLE;

	ASSERT_NO_THROW(GameManager::resetGame(
		registry,
		inputSystem,
		playerSnake,
		secondSnake,
		food,
		32,
		32,
		arena,
		aiPresets,
		mode
	));
	menuSystem.setupStartButtons(registry, menuButtons.start);
	GameEntityCounts firstCounts = countGameplayEntities(registry);

	// Simulate repeated ReturnToMenu-style path from main (reset + menu setup)
	ASSERT_NO_THROW(GameManager::resetGame(
		registry,
		inputSystem,
		playerSnake,
		secondSnake,
		food,
		32,
		32,
		arena,
		aiPresets,
		mode
	));
	menuSystem.setupStartButtons(registry, menuButtons.start);
	GameEntityCounts secondCounts = countGameplayEntities(registry);

	EXPECT_EQ(firstCounts.snakes, 1);
	EXPECT_EQ(firstCounts.food, 1);
	EXPECT_EQ(secondCounts.snakes, 1);
	EXPECT_EQ(secondCounts.food, 1);
	EXPECT_EQ(secondCounts.buttons, static_cast<int>(menuButtons.start.size()));
}

// 4 - Registry reset during transition does not leave stale references in UI/pixel text helpers
TEST(StateTransitions, RegistryResetDoesNotLeaveStaleReferences) {
	Registry registry;

	Entity uiEntity1 = registry.createEntity();
	Entity uiEntity2 = registry.createEntity();
	registry.addComponent<PixelTextComponent>(uiEntity1, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(uiEntity1, PixelTextLayoutComponent{});
	registry.addComponent<PixelTextComponent>(uiEntity2, PixelTextComponent{});

	EXPECT_TRUE(registry.hasComponent<PixelTextComponent>(uiEntity1));
	EXPECT_TRUE(registry.hasComponent<PixelTextComponent>(uiEntity2));

	// Transition-style reset used by GameManager::resetGame
	registry = Registry{};

	EXPECT_ANY_THROW(registry.getComponent<PixelTextComponent>(uiEntity1));
	EXPECT_ANY_THROW(registry.getComponent<PixelTextComponent>(uiEntity2));
}

// 5 - Re-entering Playing multiple times does not duplicate gameplay entities
TEST(StateTransitions, ReEnteringPlayingMultipleTimesDoesNotDuplicateEntities) {
	Registry registry;
	InputSystem inputSystem;
	ArenaGrid arena(32, 32);
	AIPresetLoader::PresetTable aiPresets;
	ASSERT_NO_THROW(aiPresets = AIPresetLoader::load("data/AIPresets.json"));

	Entity playerSnake(0u), secondSnake(0u), food(0u);
	GameMode mode = GameMode::SINGLE;

	for (int i = 0; i < 3; ++i) {
		ASSERT_NO_THROW(GameManager::resetGame(
			registry,
			inputSystem,
			playerSnake,
			secondSnake,
			food,
			32,
			32,
			arena,
			aiPresets,
			mode
		));

		GameEntityCounts counts = countGameplayEntities(registry);
		EXPECT_EQ(counts.snakes, 1);
		EXPECT_EQ(counts.food, 1);
	}
}
