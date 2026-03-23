#include "PixelTextHelper.hpp"
#include "../incs/Colors.hpp"
#include "../incs/RaylibColors.hpp"
#include "helpers/Factories.hpp"

bool PixelTextHelper::tryMakePresetTemplate(
	const GlyphPresetLoader::PresetTable& glyphPresets,
	const std::string& id,
	PixelTextComponent& outTemplate
) {
	auto it = glyphPresets.find(id);
	if (it == glyphPresets.end()) {
		return false;
	}

	outTemplate = it->second;
	outTemplate.visible = false;
	return true;
}

PixelTextComponent PixelTextHelper::makeGameOverTitleTemplate(
	const GlyphPresetLoader::PresetTable& glyphPresets
) {
	PixelTextComponent preset;
	if (tryMakePresetTemplate(glyphPresets, "gameover_title", preset)) {
		return preset;
	}

	PixelTextComponent fallback;
	fallback.visibleInStates.push_back(GameState::GameOver);
	fallback.id = "gameover_title";
	fallback.text = "GAME OVER";
	fallback.position = {640.0f, 180.0f};
	fallback.scale = 2.0f;
	fallback.color = customWhite;
	fallback.visible = false;
	return fallback;
}

bool PixelTextHelper::makeMenuLogoTemplate(
	const GlyphPresetLoader::PresetTable& glyphPresets,
	PixelTextComponent& outTemplate
) {
	return tryMakePresetTemplate(glyphPresets, "menu_logo", outTemplate);
}

void PixelTextHelper::applyPixelTextTemplate(
	Registry& registry,
	Entity entity,
	const PixelTextComponent& templateData
) {
	if (!registry.hasComponent<PixelTextComponent>(entity)
		|| !registry.hasComponent<PixelTextLayoutComponent>(entity)) {
		return;
	}

	auto& text = registry.getComponent<PixelTextComponent>(entity);
	auto& layout = registry.getComponent<PixelTextLayoutComponent>(entity);
	text = templateData;
	layout.dirty = true;
}

bool PixelTextHelper::shouldBeVisibleForState(
	const PixelTextComponent& text,
	GameState state
) {
	if (text.visibleInStates.empty()) {
		return text.visible;
	}

	for (GameState s : text.visibleInStates) {
		if (s == state) {
			return true;
		}
	}
	return false;
}

void PixelTextHelper::applyPixelTextStateVisibility(
	Registry& registry,
	GameState state
) {
	auto view = registry.view<PixelTextComponent, PixelTextLayoutComponent>();
	for (Entity e : view) {
		auto& text = registry.getComponent<PixelTextComponent>(e);
		auto& layout = registry.getComponent<PixelTextLayoutComponent>(e);
		const bool expectedVisible = shouldBeVisibleForState(text, state);
		if (text.visible != expectedVisible) {
			text.visible = expectedVisible;
			layout.dirty = true;
		}
	}
}

Entity PixelTextHelper::ensurePixelTextEntity(
	Registry& registry,
	Entity& entity,
	const PixelTextComponent& templateData
) {
	if (registry.hasComponent<PixelTextComponent>(entity)
		&& registry.hasComponent<PixelTextLayoutComponent>(entity)) {
		return entity;
	}

	entity = Factories::spawnPixelText(registry, templateData, true);
	return entity;
}
