#include "systems/RenderSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "components/PositionComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "Colors.hpp"
#include <rlgl.h>
#include <cmath>

// ─── Init ────────────────────────────────────────────────────────────────────

void RenderSystem::init(int w, int h) {
    gridWidth  = w;
    gridHeight = h;

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Rosario");
    ToggleFullscreen();
    SetTargetFPS(60);

    setupCamera3D();

    camera2D.offset   = { 0.0f, 0.0f };
    camera2D.target   = { 0.0f, 0.0f };
    camera2D.rotation = 0.0f;
    camera2D.zoom     = 1.0f;

    calculate2DLayout();
}

// ─── Public entry point ───────────────────────────────────────────────────────

void RenderSystem::render(Registry& registry, RenderMode mode, float deltaTime,
                          const ArenaGrid* arena) {
    accumulatedTime += deltaTime;

    BeginDrawing();
    ClearBackground(BLACK);

    if (mode == RenderMode::MODE3D) {
        render3D(registry, arena);
    } else {
        render2D(registry, arena);
    }

    EndDrawing();
}

// ─── Layout helpers ───────────────────────────────────────────────────────────

void RenderSystem::calculate2DLayout() {
    arenaWidth   = static_cast<float>((gridWidth  * squareSize) + (2 * borderThickness));
    arenaHeight  = static_cast<float>((gridHeight * squareSize) + (2 * borderThickness));
    arenaOffsetX = (screenWidth  - arenaWidth)  / 2.0f;
    arenaOffsetY = (screenHeight - arenaHeight) / 2.0f;
    gameAreaX    = arenaOffsetX + borderThickness;
    gameAreaY    = arenaOffsetY + borderThickness;
}

Vector2 RenderSystem::gridToScreen2D(int gridX, int gridY) const {
    return {
        gameAreaX + static_cast<float>(gridX * squareSize),
        gameAreaY + static_cast<float>(gridY * squareSize),
    };
}

Color RenderSystem::baseColorToRaylib(const BaseColor& c) const {
    return { c.r, c.g, c.b, c.a };
}

// ─── 2D pipeline ──────────────────────────────────────────────────────────────

void RenderSystem::render2D(Registry& registry, const ArenaGrid* arena) {
    BeginMode2D(camera2D);

    // Arena border
    DrawRectangleLinesEx(
        { arenaOffsetX, arenaOffsetY, arenaWidth, arenaHeight },
        static_cast<float>(borderThickness),
        wallColor
    );

    if (arena)
        drawArena2D(*arena);
    else
        drawWalls2D(registry);

    drawFood2D(registry);
    drawSnakes2D(registry);

    EndMode2D();
}

void RenderSystem::drawWalls2D(Registry& registry) const {
    for (auto entity : registry.view<SolidTag, PositionComponent>()) {
        const auto& pos = registry.getComponent<PositionComponent>(entity).position;
        Vector2 screen  = gridToScreen2D(pos.x, pos.y);
        DrawRectangle(
            static_cast<int>(screen.x), static_cast<int>(screen.y),
            squareSize, squareSize,
            wallColor
        );
    }
}

// Draws arena cells (Obstacle, SpawningSolid, DespawningSolid) from the ArenaGrid.
// Uses spawn/despawn fade progress for alpha to match the OOP animation.
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
                squareSize, squareSize, c
            );
        }
    }
}

void RenderSystem::drawFood2D(Registry& registry) const {
    for (auto entity : registry.view<FoodTag, PositionComponent>()) {
        const auto& pos = registry.getComponent<PositionComponent>(entity).position;
        Vector2 screen  = gridToScreen2D(pos.x, pos.y);

        float pulse = 1.0f + sinf(accumulatedTime * 3.0f) * 0.1f;
        float size  = static_cast<float>(squareSize) * 0.7f * pulse;
        float offset = (static_cast<float>(squareSize) - size) / 2.0f;

        DrawRectangle(
            static_cast<int>(screen.x + offset), static_cast<int>(screen.y + offset),
            static_cast<int>(size), static_cast<int>(size),
            foodFront
        );
    }
}

void RenderSystem::drawSnakes2D(Registry& registry) const {
    for (auto entity : registry.view<SnakeComponent, RenderComponent>()) {
        const auto& snake  = registry.getComponent<SnakeComponent>(entity);
        const auto& render = registry.getComponent<RenderComponent>(entity);
        Color color        = baseColorToRaylib(render.color);

        for (size_t i = 0; i < snake.segments.size(); ++i) {
            Vector2 screen = gridToScreen2D(snake.segments[i].position.x,
                                            snake.segments[i].position.y);
            DrawRectangle(
                static_cast<int>(screen.x), static_cast<int>(screen.y),
                squareSize, squareSize,
                color
            );
        }
    }
}

// ─── 3D pipeline ──────────────────────────────────────────────────────────────

void RenderSystem::setupCamera3D() {
    const float diagonal  = sqrtf(static_cast<float>(gridWidth  * gridWidth +
                                                       gridHeight * gridHeight)) * cubeSize;
    const float distance  = diagonal * 2.2f;
    const float elevation = 35.264f * DEG2RAD;
    const float rotation  = 45.0f  * DEG2RAD;

    camera3D.position = {
        distance * cosf(rotation) * cosf(elevation),
        distance * sinf(elevation),
        distance * sinf(rotation) * cosf(elevation)
    };
    camera3D.target     = { 0.0f, 0.0f, 0.0f };
    camera3D.up         = { 0.0f, 1.0f, 0.0f };
    camera3D.projection = CAMERA_ORTHOGRAPHIC;

    const float cameraSize = static_cast<float>((gridWidth + gridHeight) / 2);
    customFov = 0.022619f * cameraSize * cameraSize + 0.198810f * cameraSize + 31.028571f;
    camera3D.fovy = customFov;
}

void RenderSystem::render3D(Registry& registry, const ArenaGrid* arena) {
    BeginMode3D(camera3D);

    drawGroundPlane3D();
    if (arena)
        drawArena3D(*arena);
    else
        drawWalls3D(registry);
    drawFood3D(registry);
    drawSnakes3D(registry);

    EndMode3D();
}

void RenderSystem::drawGroundPlane3D() const {
    const float offsetX = (gridWidth  * cubeSize) / 2.0f;
    const float offsetZ = (gridHeight * cubeSize) / 2.0f;

    for (int z = 0; z < gridHeight; ++z) {
        for (int x = 0; x < gridWidth; ++x) {
            Vector3 pos = {
                x * cubeSize - offsetX,
                0.0f,
                z * cubeSize - offsetZ
            };
            if ((x + z) % 2 == 0) {
                drawCubeCustomFaces(pos, cubeSize, cubeSize, cubeSize,
                    groundLightFront, groundHidden, groundLightTop,
                    groundHidden, groundLightSide, groundHidden);
            } else {
                drawCubeCustomFaces(pos, cubeSize, cubeSize, cubeSize,
                    groundDarkFront, groundHidden, groundDarkTop,
                    groundHidden, groundDarkSide, groundHidden);
            }
        }
    }
}

void RenderSystem::drawWalls3D(Registry& registry) const {
    const float offsetX = (gridWidth  * cubeSize) / 2.0f;
    const float offsetZ = (gridHeight * cubeSize) / 2.0f;

    for (auto entity : registry.view<SolidTag, PositionComponent>()) {
        const auto& pos = registry.getComponent<PositionComponent>(entity).position;
        Vector3 worldPos = {
            pos.x * cubeSize - offsetX,
            cubeSize,
            pos.y * cubeSize - offsetZ
        };
        drawCubeCustomFaces(worldPos, cubeSize, cubeSize, cubeSize,
            wallColor, wallColorFade, wallColor,
            wallColorFade, wallColor, wallColorFade);
    }
}

void RenderSystem::drawArena3D(const ArenaGrid& arena) const {
    const float offsetX = (gridWidth  * cubeSize) / 2.0f;
    const float offsetZ = (gridHeight * cubeSize) / 2.0f;

    const int fw = arena.getFullWidth();
    const int fh = arena.getFullHeight();
    const auto& grid = arena.getGrid();

    for (int gy = 0; gy < fh; ++gy) {
        for (int gx = 0; gx < fw; ++gx) {
            CellType cell = grid[gy][gx];
            if (cell == CellType::Empty || cell == CellType::SpawningSolid) continue;
            if (cell == CellType::Wall) continue; // border drawn by ground plane / border box

            int gameX = gx - 1;
            int gameY = gy - 1;

            Vector3 worldPos = {
                gameX * cubeSize - offsetX,
                cubeSize,
                gameY * cubeSize - offsetZ
            };

            Color face = wallColor;
            if (cell == CellType::DespawningSolid) {
                float a = arena.getDespawnFadeProgress();
                face.a = static_cast<unsigned char>(a * 255.0f);
            } else if (cell == CellType::Obstacle && arena.isSpawning()) {
                float a = arena.getSpawnFadeProgress();
                face.a = static_cast<unsigned char>(a * 255.0f);
            }

            Color fade = wallColorFade;
            fade.a = face.a;
            drawCubeCustomFaces(worldPos, cubeSize, cubeSize, cubeSize,
                face, fade, face, fade, face, fade);
        }
    }
}

void RenderSystem::drawFood3D(Registry& registry) const {
    const float offsetX = (gridWidth  * cubeSize) / 2.0f;
    const float offsetZ = (gridHeight * cubeSize) / 2.0f;
    const float pulse   = 1.0f + sinf(accumulatedTime * 3.0f) * 0.1f;

    for (auto entity : registry.view<FoodTag, PositionComponent>()) {
        const auto& pos = registry.getComponent<PositionComponent>(entity).position;
        Vector3 worldPos = {
            pos.x * cubeSize - offsetX,
            cubeSize,
            pos.y * cubeSize - offsetZ
        };
        const float s = cubeSize * 0.7f * pulse;
        drawCubeCustomFaces(worldPos, s, s, s,
            foodFront, foodHidden, foodTop,
            foodHidden, foodSide, foodHidden);
    }
}

void RenderSystem::drawSnakes3D(Registry& registry) const {
    const float offsetX = (gridWidth  * cubeSize) / 2.0f;
    const float offsetZ = (gridHeight * cubeSize) / 2.0f;

    for (auto entity : registry.view<SnakeComponent, RenderComponent>()) {
        const auto& snake  = registry.getComponent<SnakeComponent>(entity);
        const auto& render = registry.getComponent<RenderComponent>(entity);

        // The 3D snake uses per-face color sets from RaylibColors.
        // RenderComponent.color drives which set to pick at a higher level;
        // for now the checkerboard alternates light/dark faces as in the old renderer.
        for (size_t i = 0; i < snake.segments.size(); ++i) {
            const Vec2& seg = snake.segments[i].position;
            float size      = (i == 0) ? cubeSize : cubeSize * 0.8f;
            float yPos      = (i == 0) ? cubeSize : cubeSize * 0.8f;

            Vector3 worldPos = {
                seg.x * cubeSize - offsetX,
                yPos,
                seg.y * cubeSize - offsetZ
            };

            Color hidden = snakeAHidden; // default; will be driven by RenderComponent later
            if (i % 2 == 0) {
                drawCubeCustomFaces(worldPos, size, size, size,
                    snakeALightFront, hidden, snakeALightTop,
                    hidden, snakeALightSide, hidden);
            } else {
                drawCubeCustomFaces(worldPos, size, size, size,
                    snakeADarkFront, hidden, snakeADarkTop,
                    hidden, snakeADarkSide, hidden);
            }
        }
    }
}

// ─── Low-level cube draw ──────────────────────────────────────────────────────

void RenderSystem::drawCubeCustomFaces(Vector3 position, float w, float h, float d,
                                        Color front, Color back, Color top,
                                        Color bottom, Color right, Color left) const {
    const float x = position.x;
    const float y = position.y;
    const float z = position.z;

    rlPushMatrix();
    rlTranslatef(x, y, z);
    rlBegin(RL_QUADS);

    // Front (+Z)
    rlColor4ub(front.r, front.g, front.b, front.a);
    rlVertex3f(-w/2, -h/2,  d/2);
    rlVertex3f( w/2, -h/2,  d/2);
    rlVertex3f( w/2,  h/2,  d/2);
    rlVertex3f(-w/2,  h/2,  d/2);

    // Back (-Z)
    rlColor4ub(back.r, back.g, back.b, back.a);
    rlVertex3f(-w/2, -h/2, -d/2);
    rlVertex3f(-w/2,  h/2, -d/2);
    rlVertex3f( w/2,  h/2, -d/2);
    rlVertex3f( w/2, -h/2, -d/2);

    // Top (+Y)
    rlColor4ub(top.r, top.g, top.b, top.a);
    rlVertex3f(-w/2,  h/2, -d/2);
    rlVertex3f(-w/2,  h/2,  d/2);
    rlVertex3f( w/2,  h/2,  d/2);
    rlVertex3f( w/2,  h/2, -d/2);

    // Bottom (-Y)
    rlColor4ub(bottom.r, bottom.g, bottom.b, bottom.a);
    rlVertex3f(-w/2, -h/2, -d/2);
    rlVertex3f( w/2, -h/2, -d/2);
    rlVertex3f( w/2, -h/2,  d/2);
    rlVertex3f(-w/2, -h/2,  d/2);

    // Right (+X)
    rlColor4ub(right.r, right.g, right.b, right.a);
    rlVertex3f( w/2, -h/2, -d/2);
    rlVertex3f( w/2,  h/2, -d/2);
    rlVertex3f( w/2,  h/2,  d/2);
    rlVertex3f( w/2, -h/2,  d/2);

    // Left (-X)
    rlColor4ub(left.r, left.g, left.b, left.a);
    rlVertex3f(-w/2, -h/2, -d/2);
    rlVertex3f(-w/2, -h/2,  d/2);
    rlVertex3f(-w/2,  h/2,  d/2);
    rlVertex3f(-w/2,  h/2, -d/2);

    rlEnd();
    rlPopMatrix();
}
