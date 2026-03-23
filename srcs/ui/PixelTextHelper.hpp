#pragma once

#include <string>
#include <vector>
#include "../incs/DataStructs.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "components/PixelTextComponent.hpp"
#include "components/PixelTextLayoutComponent.hpp"
#include "ui/GlyphPresetLoader.hpp"

class PixelTextHelper {
public:
	/**
	 * Load a pixel text template from the glyph presets by ID.
	 * @param glyphPresets The glyph preset table to search
	 * @param id The template ID to look up
	 * @param outTemplate Output parameter for the loaded template
	 * @return true if template found and loaded, false otherwise
	 */
	static bool tryMakePresetTemplate(
		const GlyphPresetLoader::PresetTable& glyphPresets,
		const std::string& id,
		PixelTextComponent& outTemplate
	);

	/**
	 * Create the "Game Over" title template, falling back to defaults if not in presets.
	 * @param glyphPresets The glyph preset table
	 * @return Configured PixelTextComponent for game over title
	 */
	static PixelTextComponent makeGameOverTitleTemplate(
		const GlyphPresetLoader::PresetTable& glyphPresets
	);

	/**
	 * Create the menu logo template from presets.
	 * @param glyphPresets The glyph preset table
	 * @param outTemplate Output parameter for the template
	 * @return true if template was successfully created, false otherwise
	 */
	static bool makeMenuLogoTemplate(
		const GlyphPresetLoader::PresetTable& glyphPresets,
		PixelTextComponent& outTemplate
	);

	/**
	 * Apply a template to an existing pixel text entity.
	 * @param registry The ECS registry
	 * @param entity The entity to update
	 * @param templateData The template data to apply
	 */
	static void applyPixelTextTemplate(
		Registry& registry,
		Entity entity,
		const PixelTextComponent& templateData
	);

	/**
	 * Check if a pixel text should be visible for the current game state.
	 * @param text The pixel text component
	 * @param state The current game state
	 * @return true if the text should be visible
	 */
	static bool shouldBeVisibleForState(
		const PixelTextComponent& text,
		GameState state
	);

	/**
	 * Update visibility of all pixel text entities based on game state.
	 * @param registry The ECS registry
	 * @param state The current game state
	 */
	static void applyPixelTextStateVisibility(
		Registry& registry,
		GameState state
	);

	/**
	 * Ensure a pixel text entity exists and is properly configured.
	 * Creates the entity if it doesn't exist, or validates it if it does.
	 * @param registry The ECS registry
	 * @param entity Reference to the entity (may be modified)
	 * @param templateData The template to apply
	 * @return The entity ID
	 */
	static Entity ensurePixelTextEntity(
		Registry& registry,
		Entity& entity,
		const PixelTextComponent& templateData
	);
};
