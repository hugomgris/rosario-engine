#pragma once
#include <vector>
#include <functional>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <raylib.h>
#include "arena/ArenaGrid.hpp"

// ─── TunnelLine ───────────────────────────────────────────────────────────────
//
// Represents one "ripple" line expanding outward from the arena centre to the
// wall outline.  progress 0 = just spawned near the centre, 1 = at the edge.

struct TunnelLine {
    float progress = 0.0f;  // 0→1 expansion
    float age      = 0.0f;  // seconds alive
    int   epoch    = 0;     // shape snapshot epoch (for arena transition)
};

// ─── TunnelConfig ─────────────────────────────────────────────────────────────

struct TunnelConfig {
    int   borderThickness = 20;
    int   contentInset    = 80;
    float spawnInterval   = 0.3f;
    float animationSpeed  = 0.5f;
    int   maxLines        = 8;
    Color lineColor       = { 70, 130, 180, 255 };

    // Factory presets
    static TunnelConfig menu()   { return { 20, 80,  0.30f, 0.5f,  8, { 70, 130, 180, 255 } }; }
    static TunnelConfig realm2D(){ return { 15, 40,  0.15f, 0.7f, 12, {144, 238, 144, 255 } }; }
};

// ─── ScreenShakeConfig ────────────────────────────────────────────────────────

struct ScreenShakeConfig {
    float intensity = 8.0f;
    float duration  = 0.25f;
    float frequency = 0.0f; // unused for now
};

// ─── AnimationSystem ──────────────────────────────────────────────────────────
//
// Drives the tunnel-line effect: polygon outlines of the arena walls expand
// from the centre outward as animated line strips.
//
// Usage:
//   1. Call init() once after the window is created.
//   2. Call enableTunnelEffect(true, config) to turn the effect on.
//   3. Call update(dt) and renderTunnelEffect() / renderTunnelEffectCustom()
//      each frame inside BeginDrawing()/EndDrawing().
//
// For arena preset changes:
//   - Before preset loads: notifyArenaSpawning()  — snapshots pre-spawn outlines
//   - Before preset clears: notifyArenaDespawning() — snapshots post-spawn outlines
//
class AnimationSystem {
public:
    AnimationSystem();
    ~AnimationSystem() = default;

    // Call after InitWindow()
    void init(int screenW, int screenH);

    // ── Tunnel effect ─────────────────────────────────────────────────────
    void enableTunnelEffect(bool enabled,
                            const TunnelConfig& config = TunnelConfig::menu());
    void update(float dt);

    // Basic rectangular border loop
    void renderTunnelEffect();

    // Arena-geometry-driven: outlines come from ArenaGrid::getAllOutlines()
    void renderTunnelEffectCustom(const ArenaGrid& arena,
                                  int borderLeft, int borderTop,
                                  int borderRight, int borderBottom);

    void clearTunnelEffect();

    // ── Arena transition notifications ────────────────────────────────────
    void notifyArenaSpawning();
    void notifyArenaDespawning();

    // Called when all pre-despawn lines have faded
    std::function<void()> onDespawnReadyCallback;
    bool despawnPending = false;

    // ── Screen shake ──────────────────────────────────────────────────────
    void    triggerScreenShake(const ScreenShakeConfig& config);
    void    updateScreenShake(float dt);
    Vector2 getScreenShakeOffset() const { return shakeOffset; }

    // ── Accessors ─────────────────────────────────────────────────────────
    bool isTunnelEffectEnabled() const { return tunnelEffectEnabled; }

private:
    // Configuration
    TunnelConfig        currentTunnelConfig;
    ScreenShakeConfig   shakeConfig;
    bool                tunnelEffectEnabled = false;
    float               tunnelLineThickness = 2.0f;

    // Screen size
    int screenWidth  = 1920;
    int screenHeight = 1080;

    // Active lines
    std::vector<TunnelLine> tunnelLines;
    std::vector<Vector2>    basicTunnelLine;    // used by renderTunnelEffect()

    // Epoch / snapshot system for arena transitions
    int                                currentEpoch   = 0;
    std::vector<std::vector<Vector2>>  previousShapes;
    std::vector<std::vector<Vector2>>  currentShapes;

    // Spawn timer (wall-clock based like the original)
    std::chrono::high_resolution_clock::time_point lastTunnelSpawn;

    // Screen shake
    float   shakeTimer  = 0.0f;
    Vector2 shakeOffset = { 0.0f, 0.0f };

    // ── Helpers ───────────────────────────────────────────────────────────
    float easeInQuad(float t) const { return t * t; }

    std::vector<Vector2> createRectangularShape(int l, int t, int r, int b) const;

    std::vector<Vector2> calculateInsetShape(const std::vector<Vector2>& outer,
                                             Vector2 center,
                                             float insetRatio,
                                             float maxInsetPixels) const;

    void renderTunnelLine(const TunnelLine& line,
                          const std::vector<Vector2>& outerShape,
                          Vector2 center, float maxInset) const;
};
