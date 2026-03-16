#pragma once
#include <cstddef>
#include <memory>
#include <optional>

struct Vec2 {
	int	x;
	int	y;
};

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class Input {
	None,
	Switch2D,
	Switch3D,
	Up_A,		// Player A Up
	Down_A,		// Player A Down
	Left_A,		// Player A Left
	Right_A,	// Player A Right
	Up_B,		// Player B Up
	Down_B,		// Player B Down
	Left_B,		// Player B Left
	Right_B,	// Player B Right
	Quit,
	Pause,
	Enter,
	ToggleFS
};

enum class AIBehaviourState {
    EASY, MEDIUM, HARD
};

enum class GameMode {
	SINGLE,
	MULTI,
	VSAI
};

enum class PlayerSlot {
	A,
	B,
};

enum class AppState {
	Menu,
	Playing,
	Paused,
	GameOver
};

enum class MenuAction {
	None,
	StartGame,
	SwitchMode,
	Restart,
	Quit
};