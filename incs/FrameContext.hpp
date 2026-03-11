#pragma once

// Forward declaration to avoid pulling in ArenaGrid everywhere
class ArenaGrid;

enum class RenderMode;

struct FrameContext {
	// arena
	ArenaGrid*	arena		= nullptr;
	int			gridWidth	= 0;
	int			gridHeight	= 0;

	// render
	RenderMode*	renderMode	= nullptr;

	// gameplay flags
	bool		playerDied	= false;
};
