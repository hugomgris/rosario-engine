#include <raylib.h>
#include <rlgl.h>
#include <cmath>
#include "systems/RenderSystem.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/RenderComponent.hpp"
#include "../components/SolidTag.hpp"
#include "../components/FoodTag.hpp"
#include "../../incs/Colors.hpp"

void RenderSystem::init(int gridWidth, int gridHeight) {
	_gridWidth = gridWidth;
	_gridHeight = gridHeight;

	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(_screenWidth, _screenHeight, "Rosario");
	ToggleFullscreen();
	SetTargetFPS(60);

	//setupCamera3D();

	_camera2D.offset		= { 0.0f, 0.0f };
	_camera2D.target		= { 0.0f, 0.0f };
	_camera2D.rotation	= 0.0f;
	_camera2D.zoom		= 1.0f;

	calculate2DLayout();
}

// entry point
void RenderSystem::render(Registry& registry, RenderMode mode, float deltaTime, const ArenaGrid* arena) {
	(void)mode;
	_accumulatedTime += deltaTime;

	BeginDrawing();
	ClearBackground(customBlack);

	render2D(registry, arena); // TODO: 3D pipeline and arena

	EndDrawing();
}

// helpers
void RenderSystem::calculate2DLayout() {
	_arenaWidth		= static_cast<float>((_gridWidth * _squareSize) + (2 * _borderThickness));
	_arenaHeight	= static_cast<float>((_gridHeight * _squareSize) + (2 * _borderThickness));
	_arenaOffsetX	= (_screenWidth - _arenaWidth) / 2.0f;
	_arenaOffsetY	= (_screenHeight - _arenaHeight) / 2.0f;
	_gameAreaX		= _arenaOffsetX + _borderThickness;
	_gameAreaY		= _arenaOffsetY + _borderThickness;
}

Vector2 RenderSystem::gridToScreen2D(int gridX, int gridY) const {
	return {
		_gameAreaX + static_cast<float>(gridX * _squareSize),
		_gameAreaY + static_cast<float>(gridY * _squareSize)
	};
}

Color RenderSystem::baseColorToRaylib(const BaseColor& c) const {
	return { c.r, c.g, c.b, c.a };
}

// 2D pipeline
void RenderSystem::render2D(Registry& registry, const ArenaGrid* arena) {
	BeginMode2D(_camera2D);

	// Arena Border;
	DrawRectangleLinesEx(
		{ _arenaOffsetX, _arenaOffsetY, _arenaWidth, _arenaHeight },
		static_cast<float>(_borderThickness),
		wallColor
	); 

	drawArena2D(*arena);
	//drawWalls2D(registry); // TODO: switch to arena management

	drawFood2D(registry);
	drawSnakes2D(registry);

	EndMode2D();
}

void RenderSystem::drawArena2D(const ArenaGrid& arena) const {
	const bool  spawning   = arena.isSpawning();
	const bool  despawning = arena.isDespawning();
	const float spawnAlpha = spawning   ? arena.getSpawnFadeProgress()   : 1.0f;
	const float despAlpha  = despawning ? arena.getDespawnFadeProgress() : 1.0f;

	const int fw = arena.getFullWidth();
	const int fh = arena.getFullHeight();
	const auto& grid = arena.getGrid();

	for (int gy = 0; gy < fh; ++gy) {
		for (int gx = 0; gx < fw; ++gx) {
			CellType cell = grid[gy][gx];
			if (cell == CellType::Empty || cell == CellType::SpawningSolid) continue;

			// Convert full-grid coords back to screen coords
			// Full grid: (1,1) == game (0,0)
			int gameX = gx - 1;
			int gameY = gy - 1;

			Color c = wallColor;
			if (cell == CellType::Obstacle) {
				if (spawning) c.a = static_cast<unsigned char>(spawnAlpha * 255.0f);
			} else if (cell == CellType::DespawningSolid) {
				c.a = static_cast<unsigned char>(despAlpha * 255.0f);
			} else if (cell == CellType::Wall) {
				// Border walls are drawn via DrawRectangleLinesEx — skip here
				continue;
			}

			Vector2 screen = gridToScreen2D(gameX, gameY);
			DrawRectangle(
				static_cast<int>(screen.x), static_cast<int>(screen.y),
				_squareSize, _squareSize, c
			);
		}
	}
}

void RenderSystem::drawWalls2D(Registry& registry) const {
	for (auto entity : registry.view<SolidTag, PositionComponent>()) {
		const auto& pos	= registry.getComponent<PositionComponent>(entity).position;
		Vector2 screen	= gridToScreen2D(pos.x, pos.y);
		DrawRectangle(
			static_cast<int>(screen.x), static_cast<int>(screen.y), 
			_squareSize, _squareSize,
			wallColor
		);
	}
}

//void RenderSystem::drawArena2D(const ArenaGrid& arena) const;

void RenderSystem::drawFood2D(Registry& registry) const {
	for (auto entity : registry.view<FoodTag, PositionComponent>()) {
		const auto& pos	= registry.getComponent<PositionComponent>(entity).position;
		Vector2 screen	= gridToScreen2D(pos.x, pos.y);

		float pulse		= 1.0f + sinf(_accumulatedTime * 0.3f) * 0.1f;
		float size		= static_cast<float>(_squareSize) * 0.7f * pulse;
		float offset	= (static_cast<float>(_squareSize) - size) / 2.0f;

		DrawRectangle(
			static_cast<int>(screen.x + offset), static_cast<int>(screen.y + offset),
			static_cast<int>(size), static_cast<int>(size),
			food2DColor
		);
	}
}

void RenderSystem::drawSnakes2D(Registry& registry) const {
	for (auto entity : registry.view<SnakeComponent, RenderComponent>()) {
		const auto& snake	= registry.getComponent<SnakeComponent>(entity);
		const auto& render	= registry.getComponent<RenderComponent>(entity);
		Color color			= baseColorToRaylib(render.color);

		for (size_t i = 0; i < snake.segments.size(); ++i) {
			Vector2 screen = gridToScreen2D(snake.segments[i].position.x, snake.segments[i].position.y);

			DrawRectangle(
				static_cast<int>(screen.x), static_cast<int>(screen.y),
				_squareSize, _squareSize,
				color
			);
		}
	}
}