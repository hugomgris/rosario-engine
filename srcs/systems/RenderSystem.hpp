#pragma once

#include "../ecs/Registry.hpp"
#include "../../incs/DataStructs.hpp"
#include "../../incs/RaylibColors.hpp"
#include "../../incs/Colors.hpp"
#include "../../incs/FrameContext.hpp"
#include "../arena/ArenaGrid.hpp"

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

		// 3D attributes
		float	_cubeSize			= 2.0f;
		float	_cameraSize			= 0.0f;
		float	_customFov			= 0.0f;

		// cameras
		Camera2D _camera2D = {};
		Camera3D _camera3D = {};

		// pipelines
		void render3D(Registry& registry, const FrameContext& ctx);

		// 2D helpers
		void drawSnakes2D(Registry& registry);  // non-const: places ParticleSpawnRequests
		void drawFood2D(Registry& registry) const;
		void drawArena2D(const ArenaGrid& arena) const;

		// 3D helpers
		void setupCamera3D();
		void drawArena3D(const ArenaGrid& arena) const;
		void drawGroundPlane3D() const;
		void drawWalls3D() const;
		void drawSnakes3D(Registry& registry) const;
		void drawFood3D(Registry& registry) const;
		void drawCubeCustomFaces(Vector3 position, float width, float height, float length,
								Color front, Color back, Color top, Color bottom, Color right, Color left) const;

		// utilities
		void	calculate2DLayout();
		Vector2	gridToScreen2D(int gridX, int gridY) const; // TODO: Vec2?
		Color	baseColorToRaylib(const BaseColor& c) const;

	public:
		void init(int gridWidth, int gridHeight);
		void fillContext(FrameContext& ctx) const;  // populates arenaBounds + cellSize before update phase
		void render(Registry& registry, float deltaTime, FrameContext& ctx);
		void render2D_content(Registry& registry, const FrameContext& ctx);  // draw without Mode2D bracket
		void beginMode2D() const;
		void endMode2D() const;
};