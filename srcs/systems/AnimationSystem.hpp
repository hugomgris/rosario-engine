#pragma once

#include <vector>
#include <functional>
#include <raylib.h>

struct TunnelConfig {
    int     borderThickness = 15;
    int     contentInset    = 40;
    float   spawnInterval   = 0.15f;
    float   animationSpeed  = 0.7f;
    int     maxLines        = 12;
    Color   lineColor       = { 144, 238, 144, 255 };
    float   lineThickness   = 2.0f;
};

struct TunnelLine {
    float   progress = 0.0f;   // 0 = at center (just spawned), 1 = at edge (dead)
    float   age      = 0.0f;
    int     epoch    = 0;
};

class ArenaGrid;  // forward — AnimationSystem only needs getAllOutlines + offset info

class AnimationSystem {
private:
    std::vector<TunnelLine>             _lines;
    std::vector<std::vector<Vector2>>   _previousShapes;  // shapes before last notify
    std::vector<std::vector<Vector2>>   _currentShapes;   // shapes after last notify

    TunnelConfig    _config;
    bool            _enabled        = false;
    float           _spawnTimer     = 0.0f;
    int             _currentEpoch   = 0;

    // screen layout — set once from FrameContext / init
    int     _screenWidth    = 1920;
    int     _screenHeight   = 1080;
    int     _offsetX        = 0;    // screen-space X of full-grid cell (0,0)
    int     _offsetY        = 0;    // screen-space Y of full-grid cell (0,0)
    int     _cellSize       = 32;   // pixels per grid cell

    // Easing
    float easeInQuad(float t) const { return t * t; }

    // Shape helpers
    std::vector<std::vector<Vector2>> scaleOutlines(std::vector<std::vector<Vector2>> raw) const;
    std::vector<Vector2> createRectangularShape(int left, int top, int right, int bottom) const;
    std::vector<Vector2> calculateInsetShape(const std::vector<Vector2>& outerShape,
                                              const Vector2& center,
                                              float insetRatio,
                                              float maxInsetPixels) const;
    void renderLine(const TunnelLine& line,
                    const std::vector<Vector2>& outerShape,
                    const Vector2& center,
                    float maxInset) const;

public:
    // Callback fired when all pre-despawn lines have finished — lets arena start its fade-out
    std::function<void()>   onDespawnReady;
    bool                    despawnPending = false;

    AnimationSystem() = default;

    // Call once after RenderSystem::fillContext so offsets are correct
    void init(int screenWidth, int screenHeight, int offsetX, int offsetY, int cellSize);

    void enable(bool enabled, const TunnelConfig& config);
    void disable();

    void update(float dt, const ArenaGrid& arena);
    void render() const;

    void notifyArenaSpawning(const ArenaGrid& arena);
    void notifyArenaDespawning(const ArenaGrid& arena);

    void clear();

    float getAnimationSpeed() const { return _config.animationSpeed; }
    bool  isEnabled()         const { return _enabled; }
};
