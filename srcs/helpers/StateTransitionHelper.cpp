#include "StateTransitionHelper.hpp"
#include "GameManager.hpp"
#include "../systems/ParticleSystem.hpp"
#include "../ui/MenuSystem.hpp"
#include "../systems/InputSystem.hpp"
#include "../arena/ArenaGrid.hpp"

void StateTransitionHelper::applyStateTransitionEffects(
	GameState& previousState,
	GameState currentState,
	StateTransitionContext& ctx
) {
	ctx.particleSystem.handleStateTransition(previousState, currentState);

	if (currentState == previousState) {
		return;
	}

	// Grid dimensions (match main.cpp constants)
	static constexpr int GRID_W = 32;
	static constexpr int GRID_H = 32;

	switch (currentState) {
		case GameState::Menu:
			ctx.arena.setMenuArena();
			ctx.menuSystem.setupStartButtons(ctx.registry, ctx.menuButtons.start);
			break;
		case GameState::Playing:
			ctx.arena.setGameplayArena();
			GameManager::resetGame(
				ctx.registry,
				ctx.inputSystem,
				ctx.playerSnake,
				ctx.secondSnake,
				ctx.food,
				GRID_W,
				GRID_H,
				ctx.arena,
				ctx.AIPresets,
				ctx.mode
			);
			break;
		case GameState::GameOver:
			ctx.arena.setMenuArena();
			ctx.menuSystem.setupGameOverButtons(ctx.registry, ctx.menuButtons.gameOver);
			break;
		case GameState::Paused:
		case GameState::Exiting:
			break;
	}

	previousState = currentState;
}
