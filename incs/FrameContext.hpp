#pragma once

#include "DataStructs.hpp"

// Forward declaration to avoid pulling data everywhere
class ArenaGrid;
enum class RenderMode;

struct ArenaBounds {
	float x      = 0.0f;   // screen-space left edge
	float y      = 0.0f;   // screen-space top edge
	float width  = 0.0f;
	float height = 0.0f;
};

struct FrameContext {
	// arena
	ArenaGrid*	arena		= nullptr;
	GameState*	state		= nullptr;
	int			gridWidth	= 0;
	int			gridHeight	= 0;
	ArenaBounds	arenaBounds;
	int			cellSize	= 0;   // screen pixels per grid cell
	float		gameAreaX	= 0.0f; // screen-space left edge of cell (0,0) — includes border offset
	float		gameAreaY	= 0.0f; // screen-space top edge of cell (0,0) — includes border offset

	// render
	RenderMode*	renderMode	= nullptr;

	// gameplay flags
	bool		playerDied	= false;
};
