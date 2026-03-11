#include <raylib.h>
#include <rlgl.h>
#include <cmath>
#include "systems/RenderSystem.hpp"
#include "../../incs/FrameContext.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/SnakeComponent.hpp"
#include "../components/RenderComponent.hpp"
#include "../components/AIComponent.hpp"
#include "../components/SolidTag.hpp"
#include "../components/FoodTag.hpp"
#include "../../incs/Colors.hpp"

void RenderSystem::init(int gridWidth, int gridHeight) {
	_gridWidth = gridWidth;
	_gridHeight = gridHeight;

	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(_screenWidth, _screenHeight, "Rosario");
	//ToggleFullscreen();
	SetTargetFPS(60);

	setupCamera3D();

	_camera2D.offset		= { 0.0f, 0.0f };
	_camera2D.target		= { 0.0f, 0.0f };
	_camera2D.rotation	= 0.0f;
	_camera2D.zoom		= 1.0f;

	calculate2DLayout();
}

// entry point
void RenderSystem::render(Registry& registry, float deltaTime, const FrameContext& ctx) {
	_accumulatedTime += deltaTime;

	BeginDrawing();
	ClearBackground(customBlack);

	if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D)
		render2D(registry, ctx);
	else if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE3D)
		render3D(registry, ctx);

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
void RenderSystem::render2D(Registry& registry, const FrameContext& ctx) {
	BeginMode2D(_camera2D);

	if (ctx.arena)
		drawArena2D(*ctx.arena);

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
				c.a = 255.0f;
			}

			Vector2 screen = gridToScreen2D(gameX, gameY);
			DrawRectangle(
				static_cast<int>(screen.x), static_cast<int>(screen.y),
				_squareSize, _squareSize, c
			);
		}
	}
}

void RenderSystem::drawFood2D(Registry& registry) const {
	for (auto entity : registry.view<FoodTag, PositionComponent>()) {
		const auto& pos	= registry.getComponent<PositionComponent>(entity).position;
		Vector2 screen	= gridToScreen2D(pos.x, pos.y);

		float pulse		= 1.0f + sinf(_accumulatedTime * 3.0f) * 0.1f;
		int pulseSize	= static_cast<int>(_squareSize * 0.7f * pulse);
		int offset		= (_squareSize - pulseSize) / 2;

		DrawRectangle(
			static_cast<int>(screen.x) + offset,
			static_cast<int>(screen.y) + offset,
			pulseSize, pulseSize,
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

// 3D pipeline
void RenderSystem::setupCamera3D() {
	float centerX = 0.0f;
	float centerZ = 0.0f;
	
	float diagonal = sqrtf(_gridWidth * _gridWidth + _gridHeight * _gridHeight) * _cubeSize;
	float distance = diagonal * 2.2f;		// 20% padding
	
	float elevation = 35.264f * DEG2RAD;	// Usual isometric angle
	float rotation = 45.0f * DEG2RAD;
	
	_camera3D.position = (Vector3){ 
		centerX + distance * cosf(rotation) * cosf(elevation),
		distance * sinf(elevation),
		centerZ + distance * sinf(rotation) * cosf(elevation)
	};
	
	_camera3D.target = (Vector3){ centerX, 0.0f, centerZ };
	_camera3D.up = (Vector3){ 0.0f, 1.0f, 0.0f };

	_cameraSize = static_cast<float>((_gridWidth + _gridHeight) / 2);
	_customFov = 0.022619f * _cameraSize * _cameraSize + 0.198810f * _cameraSize + 31.028571f;
	
	_camera3D.fovy = _customFov;
	_camera3D.projection = CAMERA_ORTHOGRAPHIC;
}

void RenderSystem::render3D(Registry& registry, const FrameContext& ctx) {
	BeginMode3D(_camera3D);

	/* if (ctx.arena)
		drawArena3D(*ctx.arena); */

	drawGroundPlane3D();
	drawFood3D(registry);
	drawSnakes3D(registry);

	EndMode3D();
}

void RenderSystem::drawArena3D(const ArenaGrid& arena) const {

}

void RenderSystem::drawGroundPlane3D() const {
	float offsetX = (_gridWidth * _cubeSize) / 2.0f;
	float offsetZ = (_gridHeight * _cubeSize) / 2.0f;
	
	for (int z = 0; z < _gridHeight; z++) {
		for (int x = 0; x < _gridWidth; x++) {		
			Vector3 position = {
				x * _cubeSize - offsetX,
				0.0f,
				z * _cubeSize - offsetZ
			};
			
			if ((x + z) % 2 == 0) {
				drawCubeCustomFaces(position, _cubeSize, _cubeSize, _cubeSize,
									groundLightFront, groundHidden, groundLightTop, groundHidden, groundLightSide, groundHidden);
			}
			else {
				drawCubeCustomFaces(position, _cubeSize, _cubeSize, _cubeSize,
									groundDarkFront, groundHidden, groundDarkTop, groundHidden, groundDarkSide, groundHidden);
			}
		}
	}	
}

void RenderSystem::drawSnakes3D(Registry& registry) const {
	for (auto entity : registry.view<SnakeComponent, RenderComponent>()) {
		const auto& snake = registry.getComponent<SnakeComponent>(entity);
		const auto& render = registry.getComponent<RenderComponent>(entity);

		bool isAISnake = registry.hasComponent<AIComponent>(entity);
		bool isSecondSnake = (snake.slot == PlayerSlot::B);

		float offsetX = (_gridWidth * _cubeSize) / 2.0f;
		float offsetZ = (_gridHeight * _cubeSize) / 2.0f;

		for (size_t i = 0; i < snake.segments.size(); i++) {
			Vector3 position = {
				snake.segments[i].position.x * _cubeSize - offsetX,
				_cubeSize,
				snake.segments[i].position.y * _cubeSize - offsetZ
			};

			float size = (i == 0) ? _cubeSize : _cubeSize * 0.8f;
			if (i > 0) position.y  *= 0.8f;
			
			if (i % 2 == 0) {
				if (isAISnake) {
					drawCubeCustomFaces(position, size, size, size, snakeAILightFront, snakeAHidden, snakeAILightTop, snakeAHidden, snakeAILightSide, snakeAHidden);
				} else if (isSecondSnake) {
					drawCubeCustomFaces(position, size, size, size, snakeBLightFront, snakeBHidden, snakeBLightTop, snakeBHidden, snakeBLightSide, snakeBHidden);
				} else {
					drawCubeCustomFaces(position, size, size, size, snakeALightFront, snakeAHidden, snakeALightTop, snakeAHidden, snakeALightSide, snakeAHidden);
				}
			} else {
				if (isAISnake) {
					drawCubeCustomFaces(position, size, size, size, snakeAIDarkFront, snakeAHidden, snakeAIDarkTop, snakeAHidden, snakeAIDarkSide, snakeAHidden);
				} else if (isSecondSnake) {
					drawCubeCustomFaces(position, size, size, size, snakeBDarkFront, snakeBHidden, snakeBDarkTop, snakeBHidden, snakeBDarkSide, snakeBHidden);
				} else {
					drawCubeCustomFaces(position, size, size, size, snakeADarkFront, snakeAHidden, snakeADarkTop, snakeAHidden, snakeADarkSide, snakeAHidden);
				}
			}
		}
	}
}

void RenderSystem::drawFood3D(Registry& registry) const {
	for (auto entity : registry.view<FoodTag, PositionComponent>()) {
		const auto& pos = registry.getComponent<PositionComponent>(entity).position;

		float offsetX = (_gridWidth * _cubeSize) / 2.0f;
		float offsetZ = (_gridHeight * _cubeSize) / 2.0f;

		Vector3 foodPos = {
			pos.x * _cubeSize - offsetX + (_cubeSize * 0.1f),
			_cubeSize,
			pos.y * _cubeSize - offsetZ + (_cubeSize * 0.1f)
		};

		float pulse = 1.0f + sinf(_accumulatedTime * 3.0f) * 0.1f;
		float size = _cubeSize * 0.7f * pulse;

		drawCubeCustomFaces(foodPos, size, size, size, foodFront, foodHidden, foodTop, foodHidden, foodSide, foodHidden);
	}
}

void RenderSystem::drawCubeCustomFaces(Vector3 position, float width, float height, float length,
										Color front, Color back, Color top, Color bottom, Color right, Color left) const {
	float x = position.x;
	float y = position.y;
	float z = position.z;
	
	// In isometric view, default visible faces are: front (+Z), top (+Y), right (+X)
	rlPushMatrix();
	rlTranslatef(x, y, z);
	
	rlBegin(RL_QUADS);
	
	// Front face (+Z) - visible
	rlColor4ub(front.r, front.g, front.b, front.a);
	rlVertex3f(-width/2, -height/2, length/2);
	rlVertex3f(width/2, -height/2, length/2);
	rlVertex3f(width/2, height/2, length/2);
	rlVertex3f(-width/2, height/2, length/2);
	
	// Back face (-Z)
	rlColor4ub(back.r, back.g, back.b, back.a);
	rlVertex3f(-width/2, -height/2, -length/2);
	rlVertex3f(-width/2, height/2, -length/2);
	rlVertex3f(width/2, height/2, -length/2);
	rlVertex3f(width/2, -height/2, -length/2);
	
	// Top face (+Y) - visible
	rlColor4ub(top.r, top.g, top.b, top.a);
	rlVertex3f(-width/2, height/2, -length/2);
	rlVertex3f(-width/2, height/2, length/2);
	rlVertex3f(width/2, height/2, length/2);
	rlVertex3f(width/2, height/2, -length/2);
	
	// Bottom face (-Y)
	rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a);
	rlVertex3f(-width/2, -height/2, -length/2);
	rlVertex3f(width/2, -height/2, -length/2);
	rlVertex3f(width/2, -height/2, length/2);
	rlVertex3f(-width/2, -height/2, length/2);
	
	// Right face (+X) - visible
	rlColor4ub(right.r, right.g, right.b, right.a);
	rlVertex3f(width/2, -height/2, -length/2);
	rlVertex3f(width/2, height/2, -length/2);
	rlVertex3f(width/2, height/2, length/2);
	rlVertex3f(width/2, -height/2, length/2);
	
	// Left face (-X)
	rlColor4ub(left.r, left.g, left.b, left.a);
	rlVertex3f(-width/2, -height/2, -length/2);
	rlVertex3f(-width/2, -height/2, length/2);
	rlVertex3f(-width/2, height/2, length/2);
	rlVertex3f(-width/2, height/2, -length/2);
	
	rlEnd();
	rlPopMatrix();
}