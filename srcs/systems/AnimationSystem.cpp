#include "AnimationSystem.hpp"
#include "../arena/ArenaGrid.hpp"
#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cmath>


void AnimationSystem::init(int screenWidth, int screenHeight, int offsetX, int offsetY, int cellSize) {
    _screenWidth  = screenWidth;
    _screenHeight = screenHeight;
    _offsetX      = offsetX;
    _offsetY      = offsetY;
    _cellSize     = cellSize;
}

void AnimationSystem::enable(bool enabled, const TunnelConfig& config) {
    _enabled = enabled;
    _config  = config;
    if (!enabled)
        clear();
}

void AnimationSystem::disable() {
    _enabled = false;
    clear();
}

void AnimationSystem::clear() {
    _lines.clear();
    _spawnTimer = 0.0f;
}


void AnimationSystem::update(float dt, const ArenaGrid& arena) {
    if (!_enabled) return;

    for (auto& line : _lines) {
        line.age     += dt;
        line.progress = easeInQuad(line.age * _config.animationSpeed);
    }

    _lines.erase(
        std::remove_if(_lines.begin(), _lines.end(),
            [](const TunnelLine& l) { return l.progress >= 1.0f; }),
        _lines.end()
    );

    if (despawnPending) {
        bool oldLinesAlive = false;
        for (const auto& l : _lines) {
            if (l.epoch < _currentEpoch) { oldLinesAlive = true; break; }
        }
        if (!oldLinesAlive) {
            despawnPending = false;
            if (onDespawnReady)
                onDespawnReady();
        }
    }

    _spawnTimer += dt;
    if (_spawnTimer >= _config.spawnInterval) {
        _spawnTimer = 0.0f;
        if (static_cast<int>(_lines.size()) < _config.maxLines) {
            TunnelLine newLine;
            newLine.epoch = _currentEpoch;
            _lines.push_back(newLine);
        }
    }

    _currentShapes = scaleOutlines(arena.getAllOutlines(_offsetX, _offsetY));
}

void AnimationSystem::render() const {
    if (!_enabled || _lines.empty()) return;

    Vector2 center = {
        static_cast<float>(_screenWidth)  / 2.0f,
        static_cast<float>(_screenHeight) / 2.0f
    };

    float maxInset = static_cast<float>(_config.contentInset);

    for (const auto& line : _lines) {
        const auto& shapes = (line.epoch < _currentEpoch) ? _previousShapes : _currentShapes;

        for (const auto& shape : shapes) {
            if (shape.empty()) continue;
            renderLine(line, shape, center, maxInset);
        }
    }
}

void AnimationSystem::notifyArenaSpawning(const ArenaGrid& arena) {
    _previousShapes = _currentShapes;
    _currentEpoch++;

    for (auto& l : _lines)
        l.epoch = _currentEpoch - 1;

    _currentShapes = scaleOutlines(arena.getAllOutlines(_offsetX, _offsetY));
}

void AnimationSystem::notifyArenaDespawning(const ArenaGrid& arena) {
    _previousShapes = _currentShapes;
    _currentEpoch++;

    for (auto& l : _lines)
        l.epoch = _currentEpoch - 1;

    _currentShapes = scaleOutlines(arena.getAllOutlines(_offsetX, _offsetY));
    despawnPending = true;
}

std::vector<std::vector<Vector2>> AnimationSystem::scaleOutlines(std::vector<std::vector<Vector2>> raw) const {
    const float ox = static_cast<float>(_offsetX);
    const float oy = static_cast<float>(_offsetY);
    const float cs = static_cast<float>(_cellSize);

    for (auto& shape : raw)
        for (auto& p : shape) {
            p.x = ox + (p.x - ox) * cs;
            p.y = oy + (p.y - oy) * cs;
        }
    return raw;
}

std::vector<Vector2> AnimationSystem::createRectangularShape(
        int left, int top, int right, int bottom) const {
    return {
        { static_cast<float>(left),  static_cast<float>(top)    },
        { static_cast<float>(right), static_cast<float>(top)    },
        { static_cast<float>(right), static_cast<float>(bottom) },
        { static_cast<float>(left),  static_cast<float>(bottom) }
    };
}

std::vector<Vector2> AnimationSystem::calculateInsetShape(
        const std::vector<Vector2>& outerShape,
        const Vector2& center,
        float insetRatio,
        float maxInsetPixels) const {
    if (outerShape.empty()) return {};

    float refDist = std::sqrt(
        (center.x * 2.0f) * (center.x * 2.0f) +
        (center.y * 2.0f) * (center.y * 2.0f)
    ) / 2.0f;

    if (refDist < 0.001f) return outerShape;

    float minScale = std::max(0.0f, (refDist - maxInsetPixels) / refDist);
    float scale    = minScale + (1.0f - minScale) * (1.0f - insetRatio);

    std::vector<Vector2> inset;
    inset.reserve(outerShape.size());

    for (const Vector2& p : outerShape)
        inset.push_back({
            center.x + (p.x - center.x) * scale,
            center.y + (p.y - center.y) * scale
        });

    return inset;
}

void AnimationSystem::renderLine(const TunnelLine& line,
                                  const std::vector<Vector2>& outerShape,
                                  const Vector2& center,
                                  float maxInset) const {
    float insetRatio = 1.0f - line.progress;

    std::vector<Vector2> shape = calculateInsetShape(outerShape, center, insetRatio, maxInset);

    unsigned char alpha  = static_cast<unsigned char>(line.progress * 255.0f);
    Color         color  = _config.lineColor;
    color.a              = alpha;

    for (size_t i = 0; i < shape.size(); ++i) {
        size_t j = (i + 1) % shape.size();
        DrawLineEx(shape[i], shape[j], _config.lineThickness, color);
    }
}
