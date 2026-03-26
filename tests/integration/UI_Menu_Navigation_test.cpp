#include <gtest/gtest.h>

#include <memory>

#include "ecs/Registry.hpp"
#include "animations/ParticleConfigLoader.hpp"
#include "ui/MenuSystem.hpp"
#include "ui/UISystem.hpp"
#include "ui/ButtonConfigLoader.hpp"
#include "ui/GlyphLibraryLoader.hpp"
#include "ui/GlyphPresetLoader.hpp"
#include "ui/PixelTextHelper.hpp"
#include "ui/PixelTextLayoutSystem.hpp"
#include "components/ButtonComponent.hpp"
#include "components/ButtonActionComponent.hpp"
#include "components/UIInteractableComponent.hpp"
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
#include "components/ParticleSpawnRequest.hpp"
#include "FrameContext.hpp"
#include "systems/UIInteractionSystem.hpp"
#include "ui/EventQueue.hpp"
#include "helpers/MenuLogoParticleHelper.hpp"

namespace {
	enum class InputSource { Keyboard, Mouse };
}

// 1 - Menu keyboard/mouse co-navigation preserves single hover source of truth across frames
TEST(UIMenuNavigation, KeyboardMouseCoNavigationPreservesSingleHoverSource) {
	Registry registry;
	UIInteractionSystem uiSystem;
	
	// Load UI configuration
	GlyphLibrary glyphLibrary;
	EXPECT_NO_THROW(glyphLibrary = GlyphLibraryLoader::load("data/GlyphLibrary.json"));
	
	GlyphPresetLoader::PresetTable glyphPresets;
	EXPECT_NO_THROW(glyphPresets = GlyphPresetLoader::load("data/GlyphPresets.json"));
	
	ButtonConfigLoader::ButtonTable buttonTable;
	EXPECT_NO_THROW(buttonTable = ButtonConfigLoader::load("data/ButtonConfig.json"));
	
	// Create a few menu buttons that participate in UIInteractionSystem ordering.
	Entity button1 = registry.createEntity();
	Entity button2 = registry.createEntity();
	Entity button3 = registry.createEntity();
	
	ButtonComponent b1;
	b1.id = "start";
	b1.config.id = "start";
	b1.config.menu = ButtonMenu::Start;
	b1.config.index = 0;
	b1.bounds = Rectangle{20.0f, 20.0f, 120.0f, 40.0f};

	ButtonComponent b2;
	b2.id = "modeChange";
	b2.config.id = "modeChange";
	b2.config.menu = ButtonMenu::Start;
	b2.config.index = 1;
	b2.bounds = Rectangle{180.0f, 20.0f, 120.0f, 40.0f};

	ButtonComponent b3;
	b3.id = "quit";
	b3.config.id = "quit";
	b3.config.menu = ButtonMenu::Start;
	b3.config.index = 2;
	b3.bounds = Rectangle{340.0f, 20.0f, 120.0f, 40.0f};

	registry.addComponent<ButtonComponent>(button1, b1);
	registry.addComponent<ButtonComponent>(button2, b2);
	registry.addComponent<ButtonComponent>(button3, b3);

	registry.addComponent<ButtonActionComponent>(button1, ButtonActionComponent{ButtonActionType::StartGame});
	registry.addComponent<ButtonActionComponent>(button2, ButtonActionComponent{ButtonActionType::ChangeMode});
	registry.addComponent<ButtonActionComponent>(button3, ButtonActionComponent{ButtonActionType::Quit});
	
	registry.addComponent<UIInteractableComponent>(button1, UIInteractableComponent{});
	registry.addComponent<UIInteractableComponent>(button2, UIInteractableComponent{});
	registry.addComponent<UIInteractableComponent>(button3, UIInteractableComponent{});
	
	// Frame 1: keyboard navigation picks button1 as hovered.
	uiSystem.setHoveredForTests(registry, ButtonMenu::Start, button1);
	EXPECT_TRUE(registry.getComponent<ButtonComponent>(button1).hovered);
	EXPECT_FALSE(registry.getComponent<ButtonComponent>(button2).hovered);
	EXPECT_FALSE(registry.getComponent<ButtonComponent>(button3).hovered);

	// Frame 2: mouse source selects button2 -> hover transfers and remains unique.
	uiSystem.setHoveredForTests(registry, ButtonMenu::Start, button2);

	int hoveredCount = 0;
	if (registry.getComponent<ButtonComponent>(button1).hovered) hoveredCount++;
	if (registry.getComponent<ButtonComponent>(button2).hovered) hoveredCount++;
	if (registry.getComponent<ButtonComponent>(button3).hovered) hoveredCount++;

	EXPECT_EQ(hoveredCount, 1);
	EXPECT_FALSE(registry.getComponent<ButtonComponent>(button1).hovered);
	EXPECT_TRUE(registry.getComponent<ButtonComponent>(button2).hovered);
	EXPECT_FALSE(registry.getComponent<ButtonComponent>(button3).hovered);
}

// 2 - State-scoped pixel text visibility toggles correctly between Menu and GameOver
TEST(UIMenuNavigation, PixelTextVisibilityTogglesPerState) {
	Registry registry;

	// Menu-scoped text
	Entity menuText = registry.createEntity();
	registry.addComponent<PixelTextComponent>(menuText, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(menuText, PixelTextLayoutComponent{});
	registry.getComponent<PixelTextComponent>(menuText).id = "menu_title";
	registry.getComponent<PixelTextComponent>(menuText).text = "Main Menu";
	registry.getComponent<PixelTextComponent>(menuText).visibleInStates = {GameState::Menu};
	registry.getComponent<PixelTextComponent>(menuText).visible = false;

	// GameOver-scoped text
	Entity gameOverText = registry.createEntity();
	registry.addComponent<PixelTextComponent>(gameOverText, PixelTextComponent{});
	registry.addComponent<PixelTextLayoutComponent>(gameOverText, PixelTextLayoutComponent{});
	registry.getComponent<PixelTextComponent>(gameOverText).id = "gameover_title";
	registry.getComponent<PixelTextComponent>(gameOverText).text = "Game Over";
	registry.getComponent<PixelTextComponent>(gameOverText).visibleInStates = {GameState::GameOver};
	registry.getComponent<PixelTextComponent>(gameOverText).visible = false;

	registry.getComponent<PixelTextLayoutComponent>(menuText).dirty = false;
	registry.getComponent<PixelTextLayoutComponent>(gameOverText).dirty = false;

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::Menu);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(menuText).visible);
	EXPECT_FALSE(registry.getComponent<PixelTextComponent>(gameOverText).visible);
	EXPECT_TRUE(registry.getComponent<PixelTextLayoutComponent>(menuText).dirty);
	EXPECT_FALSE(registry.getComponent<PixelTextLayoutComponent>(gameOverText).dirty);

	registry.getComponent<PixelTextLayoutComponent>(menuText).dirty = false;
	registry.getComponent<PixelTextLayoutComponent>(gameOverText).dirty = false;

	PixelTextHelper::applyPixelTextStateVisibility(registry, GameState::GameOver);
	EXPECT_FALSE(registry.getComponent<PixelTextComponent>(menuText).visible);
	EXPECT_TRUE(registry.getComponent<PixelTextComponent>(gameOverText).visible);
	EXPECT_TRUE(registry.getComponent<PixelTextLayoutComponent>(menuText).dirty);
	EXPECT_TRUE(registry.getComponent<PixelTextLayoutComponent>(gameOverText).dirty);

	EXPECT_FALSE(registry.getComponent<PixelTextComponent>(menuText).text.empty());
	EXPECT_TRUE(registry.hasComponent<PixelTextComponent>(menuText));
	EXPECT_TRUE(registry.hasComponent<PixelTextComponent>(gameOverText));
}

// 3 - Menu logo trail requests follow text visibility and stop when logo is hidden
TEST(UIMenuNavigation, MenuLogoTrailFollowsTextVisibility) {
	Registry registry;

	ParticleConfig cfg;
	MenuTrailPreset preset;
	preset.direction = Direction::RIGHT;
	preset.color = Color{255, 255, 255, 255};
	preset.spawnInterval = 0.12f;
	cfg.menuTrails.push_back(preset);

	// Create logo entity with text
	Entity logo = registry.createEntity();
	registry.addComponent<PixelTextComponent>(logo, PixelTextComponent{});

	PixelTextLayoutComponent logoLayout;
	logoLayout.quads.push_back(PixelTextQuad{Rectangle{100.0f, 100.0f, 30.0f, 12.0f}, WHITE});
	registry.addComponent<PixelTextLayoutComponent>(logo, logoLayout);

	auto& logoText = registry.getComponent<PixelTextComponent>(logo);
	logoText.text = "ROSARIO";
	logoText.visible = true;

	std::vector<Entity> emitters;
	ASSERT_TRUE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, logo, cfg, emitters));
	ASSERT_EQ(emitters.size(), 1u);
	ASSERT_TRUE(registry.hasComponent<ParticleSpawnRequest>(emitters[0]));
	const auto reqVisible = registry.getComponent<ParticleSpawnRequest>(emitters[0]);
	EXPECT_FLOAT_EQ(reqVisible.spawnInterval, 0.12f);
	
	// Logo should be visible initially
	EXPECT_FALSE(logoText.text.empty());
	
	// When logo is hidden, helper should stop emitting new requests
	logoText.visible = false;
	EXPECT_FALSE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, logo, cfg, emitters));
	
	// Existing request remains unchanged and no new emitter is created.
	ASSERT_EQ(emitters.size(), 1u);
	ASSERT_TRUE(registry.hasComponent<ParticleSpawnRequest>(emitters[0]));
	const auto reqHidden = registry.getComponent<ParticleSpawnRequest>(emitters[0]);
	EXPECT_FLOAT_EQ(reqHidden.spawnInterval, reqVisible.spawnInterval);
	
	// Make logo visible again
	logoText.visible = true;
	ASSERT_TRUE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, logo, cfg, emitters));
	EXPECT_FALSE(logoText.text.empty());
}

// 4 - Multiple menuTrail emitters run independently with per-emitter interval gating
TEST(UIMenuNavigation, MultipleMenuTrailEmittersRunIndependently) {
	Registry registry;

	Entity logo = registry.createEntity();
	PixelTextComponent logoText;
	logoText.id = "menu_logo";
	logoText.text = "ROSARIO";
	logoText.visible = true;
	registry.addComponent<PixelTextComponent>(logo, logoText);

	PixelTextLayoutComponent logoLayout;
	logoLayout.quads.push_back(PixelTextQuad{Rectangle{50.0f, 70.0f, 40.0f, 20.0f}, WHITE});
	registry.addComponent<PixelTextLayoutComponent>(logo, logoLayout);

	ParticleConfig cfg;
	MenuTrailPreset t1;
	t1.direction = Direction::UP;
	t1.color = Color{255, 0, 0, 255};
	t1.spawnInterval = 0.05f;

	MenuTrailPreset t2;
	t2.direction = Direction::DOWN;
	t2.color = Color{0, 255, 0, 255};
	t2.spawnInterval = 0.10f;

	MenuTrailPreset t3;
	t3.direction = Direction::LEFT;
	t3.color = Color{0, 0, 255, 255};
	t3.spawnInterval = 0.08f;

	cfg.menuTrails = {t1, t2, t3};

	std::vector<Entity> emitters;
	ASSERT_TRUE(MenuLogoParticleHelper::enqueueMenuLogoTrailRequests(registry, logo, cfg, emitters));
	ASSERT_EQ(emitters.size(), 3u);

	const auto& r1 = registry.getComponent<ParticleSpawnRequest>(emitters[0]);
	const auto& r2 = registry.getComponent<ParticleSpawnRequest>(emitters[1]);
	const auto& r3 = registry.getComponent<ParticleSpawnRequest>(emitters[2]);

	EXPECT_EQ(r1.emitterKey, 1u);
	EXPECT_EQ(r2.emitterKey, 2u);
	EXPECT_EQ(r3.emitterKey, 3u);
	EXPECT_FLOAT_EQ(r1.spawnInterval, 0.05f);
	EXPECT_FLOAT_EQ(r2.spawnInterval, 0.10f);
	EXPECT_FLOAT_EQ(r3.spawnInterval, 0.08f);
	EXPECT_EQ(r1.direction, Direction::UP);
	EXPECT_EQ(r2.direction, Direction::DOWN);
	EXPECT_EQ(r3.direction, Direction::LEFT);
}