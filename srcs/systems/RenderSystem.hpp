#pragma once

#include "../ecs/Registry.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/RaylibColors.hpp"
#include "../../incs/Colors.hpp"

enum class RenderMode {
	MODE2D,
	MODE3D,
};

class RenderSystem {
	private:
		// layout
		int		_gridWidth			= 0;
		int		_gridHeight			= 0;
		int		_screenWidth		= 1920;
		int		_screenHeight		= 1080;
		int		_squareSize			= 32;
		int		_borderThickness	= 32;
		float	_arenaOffsetX		= 0.0f;
		float	_arenaOffsetY		= 0.0f;
		float	_arenaWidth			= 0.0f;
		float	_arenaHeight		= 0.0f;
		float	_gameAreaX			= 0.0f;
		float	_gameAreaY			= 0.0f;
		float	_accumulatedTime	= 0.0f;

		// cameras
		Camera2D _camera2D = {};

		// pipelines
		void render2D(Registry& registry);
		//void render2D(Registry& registry);

		// 2D helpers
		void drawSnakes2D(Registry& registry) const;
		void drawFood2D(Registry& registry) const;
		void drawWalls2D(Registry  &registry) const;
		//void drawArena2D(const ArenaGrid& arena) const;

		// utilities
		void	calculate2DLayout();
		Vector2	gridToScreen2D(int gridX, int gridY) const; // TODO: Vec2?
		Color	baseColorToRaylib(const BaseColor& c) const;

	public:
		void init(int gridWidth, int gridHeight);
		void render(Registry& registry, RenderMode mode, float deltaTime);
};