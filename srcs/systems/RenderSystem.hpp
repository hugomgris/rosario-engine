#pragma once

#include <raylib.h>
#include "ecs/Registry.hpp"
#include "arena/ArenaGrid.hpp"
#include "DataStructs.hpp"
#include "RaylibColors.hpp"

enum class RenderMode {
    MODE2D,
    MODE3D,
};

class RenderSystem {
public:
    RenderSystem() = default;

    // Must be called once before any rendering, after the Registry is populated.
    void init(int gridWidth, int gridHeight);

    // arena may be nullptr — falls back to SolidTag entity draw for walls
    void render(Registry& registry, RenderMode mode, float deltaTime,
                const ArenaGrid* arena = nullptr);

private:
    // ── Layout ───────────────────────────────────────────────────────────────
    int   gridWidth      = 0;
    int   gridHeight     = 0;
    int   screenWidth    = 1920;
    int   screenHeight   = 1080;
    int   squareSize     = 32;
    int   borderThickness = 32;
    float arenaOffsetX   = 0.0f;
    float arenaOffsetY   = 0.0f;
    float arenaWidth     = 0.0f;
    float arenaHeight    = 0.0f;
    float gameAreaX      = 0.0f;
    float gameAreaY      = 0.0f;
    float accumulatedTime = 0.0f;

    // ── 3D ───────────────────────────────────────────────────────────────────
    Camera3D camera3D  = {};
    float    cubeSize  = 2.0f;
    float    customFov = 0.0f;

    // ── 2D ───────────────────────────────────────────────────────────────────
    Camera2D camera2D  = {};

    // ── Pipelines ────────────────────────────────────────────────────────────
    void render2D(Registry& registry, const ArenaGrid* arena);
    void render3D(Registry& registry, const ArenaGrid* arena);

    // ── 2D draw helpers ──────────────────────────────────────────────────────
    void drawSnakes2D(Registry& registry) const;
    void drawFood2D(Registry& registry) const;
    void drawWalls2D(Registry& registry) const;
    void drawArena2D(const ArenaGrid& arena) const;

    // ── 3D draw helpers ──────────────────────────────────────────────────────
    void setupCamera3D();
    void drawGroundPlane3D() const;
    void drawSnakes3D(Registry& registry) const;
    void drawFood3D(Registry& registry) const;
    void drawWalls3D(Registry& registry) const;
    void drawArena3D(const ArenaGrid& arena) const;

    // Low-level per-face cube draw (preserved from old renderer)
    void drawCubeCustomFaces(Vector3 position, float w, float h, float d,
                             Color front, Color back, Color top,
                             Color bottom, Color right, Color left) const;

    // ── Utilities ────────────────────────────────────────────────────────────
    void     calculate2DLayout();
    Vector2  gridToScreen2D(int gridX, int gridY) const;
    Color    baseColorToRaylib(const struct BaseColor& c) const;
};
