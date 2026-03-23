#pragma once

#include <vector>
#include "../incs/DataStructs.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "ui/ButtonConfigLoader.hpp"
#include "AI/AIPresetLoader.hpp"

class ParticleSystem;
class MenuSystem;
class InputSystem;
class ArenaGrid;

struct StateTransitionContext {
	ParticleSystem& particleSystem;
	MenuSystem& menuSystem;
	Registry& registry;
	const struct ButtonConfigLoader::ButtonTable& menuButtons;
	InputSystem& inputSystem;
	Entity& playerSnake;
	Entity& secondSnake;
	Entity& food;
	ArenaGrid& arena;
	const AIPresetLoader::PresetTable& AIPresets;
	GameMode& mode;
};

/**
 * Helper class for managing state transitions and their side effects.
 */
class StateTransitionHelper {
public:
	/**
	 * Apply state transition effects based on state change.
	 * Handles arena setup, menu button configuration, game reset, etc.
	 * @param previousState Reference to the previous state (will be updated)
	 * @param currentState The new state to transition to
	 * @param ctx The transition context containing all systems and entities
	 */
	static void applyStateTransitionEffects(
		GameState& previousState,
		GameState currentState,
		StateTransitionContext& ctx
	);
};
