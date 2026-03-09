#include "systems/AnimationSystem.hpp"
#include <cstdlib>
#include <cmath>

// ─── Lifecycle ────────────────────────────────────────────────────────────────

AnimationSystem::AnimationSystem()
    : lastTunnelSpawn(std::chrono::high_resolution_clock::now()) {}

void AnimationSystem::init(int sw, int sh) {
    screenWidth  = sw;
    screenHeight = sh;
}

// ─── Enable / disable ────────────────────────────────────────────────────────

void AnimationSystem::enableTunnelEffect(bool enabled, const TunnelConfig& config) {
    tunnelEffectEnabled   = enabled;
    currentTunnelConfig   = config;
    if (!enabled) clearTunnelEffect();
}

void AnimationSystem::clearTunnelEffect() {
    tunnelLines.clear();
    lastTunnelSpawn = std::chrono::high_resolution_clock::now();
}

// ─── Update ───────────────────────────────────────────────────────────────────

void AnimationSystem::update(float dt) {
    if (!tunnelEffectEnabled) return;

    // Advance all lines
    for (auto& line : tunnelLines) {
        line.age      += dt;
        line.progress  = easeInQuad(line.age * currentTunnelConfig.animationSpeed);
    }

    // Prune lines that have finished expanding
    tunnelLines.erase(
        std::remove_if(tunnelLines.begin(), tunnelLines.end(),
                       [](const TunnelLine& l) { return l.progress >= 1.0f; }),
        tunnelLines.end()
    );

    // Check if all pre-despawn lines have cleared
    if (despawnPending) {
        bool oldAlive = false;
        for (const auto& l : tunnelLines)
            if (l.epoch < currentEpoch) { oldAlive = true; break; }
        if (!oldAlive) {
            despawnPending = false;
            if (onDespawnReadyCallback) onDespawnReadyCallback();
        }
    }

    // Spawn new lines at the configured interval
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - lastTunnelSpawn;
    if (elapsed.count() >= currentTunnelConfig.spawnInterval) {
        if (static_cast<int>(tunnelLines.size()) < currentTunnelConfig.maxLines) {
            TunnelLine nl;
            nl.epoch = currentEpoch;
            tunnelLines.push_back(nl);
        }
        lastTunnelSpawn = now;
    }

    updateScreenShake(dt);
}

// ─── Render (rectangular border) ─────────────────────────────────────────────

void AnimationSystem::renderTunnelEffect() {
    if (!tunnelEffectEnabled || tunnelLines.empty()) return;

    int bt = currentTunnelConfig.borderThickness;
    basicTunnelLine = createRectangularShape(bt, bt, screenWidth - bt, screenHeight - bt);

    Vector2 center = {
        static_cast<float>(screenWidth  / 2),
        static_cast<float>(screenHeight / 2)
    };

    for (const auto& line : tunnelLines)
        renderTunnelLine(line, basicTunnelLine, center,
                         static_cast<float>(currentTunnelConfig.contentInset));
}

// ─── Render (arena-geometry driven) ──────────────────────────────────────────
//
// Gets polygon outlines from the ArenaGrid, then draws each active tunnel line
// against every outline in the relevant epoch snapshot.
//
void AnimationSystem::renderTunnelEffectCustom(const ArenaGrid& arena,
                                               int borderLeft, int borderTop,
                                               int borderRight, int borderBottom) {
    if (!tunnelEffectEnabled || tunnelLines.empty()) return;

    // Refresh current outlines every frame so newly-spawned obstacles appear
    currentShapes = arena.getAllOutlines(borderLeft, borderTop);

    Vector2 arenaCenter = {
        static_cast<float>(borderLeft + borderRight)  / 2.0f,
        static_cast<float>(borderTop  + borderBottom) / 2.0f
    };

    float maxInset = static_cast<float>(currentTunnelConfig.contentInset);

    for (const auto& line : tunnelLines) {
        // Lines born before the current epoch use the pre-change outline snapshot
        const auto& shapes = (line.epoch < currentEpoch)
                           ? previousShapes
                           : currentShapes;

        for (const auto& shape : shapes) {
            if (shape.empty()) continue;
            renderTunnelLine(line, shape, arenaCenter, maxInset);
        }
    }
}

// ─── Arena transition notifications ──────────────────────────────────────────

void AnimationSystem::notifyArenaSpawning() {
    previousShapes = currentShapes;
    ++currentEpoch;
    for (auto& l : tunnelLines)
        l.epoch = currentEpoch - 1;
}

void AnimationSystem::notifyArenaDespawning() {
    previousShapes = currentShapes;
    ++currentEpoch;
    for (auto& l : tunnelLines)
        l.epoch = currentEpoch - 1;
    despawnPending = true;
}

// ─── Screen shake ─────────────────────────────────────────────────────────────

void AnimationSystem::triggerScreenShake(const ScreenShakeConfig& config) {
    shakeConfig = config;
    shakeTimer  = config.duration;
}

void AnimationSystem::updateScreenShake(float dt) {
    if (shakeTimer <= 0.0f) {
        shakeOffset = { 0.0f, 0.0f };
        return;
    }
    shakeTimer -= dt;
    float intensity = shakeConfig.intensity * (shakeTimer / shakeConfig.duration);
    shakeOffset.x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * intensity;
    shakeOffset.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * intensity;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

std::vector<Vector2> AnimationSystem::createRectangularShape(
        int l, int t, int r, int b) const {
    return {
        { static_cast<float>(l), static_cast<float>(t) },
        { static_cast<float>(r), static_cast<float>(t) },
        { static_cast<float>(r), static_cast<float>(b) },
        { static_cast<float>(l), static_cast<float>(b) },
    };
}

// Scale every outline point toward arenaCenter by an amount determined by
// insetRatio: 0 = at edge, 1 = fully at centre.
std::vector<Vector2> AnimationSystem::calculateInsetShape(
        const std::vector<Vector2>& outer,
        Vector2 center,
        float insetRatio,
        float maxInsetPixels) const {
    if (outer.empty()) return {};

    float refDist = sqrtf(
        (center.x * 2.0f) * (center.x * 2.0f) +
        (center.y * 2.0f) * (center.y * 2.0f)
    ) / 2.0f;

    if (refDist < 0.001f) return outer;

    float minScale = std::max(0.0f, (refDist - maxInsetPixels) / refDist);
    float scale    = minScale + (1.0f - minScale) * (1.0f - insetRatio);

    std::vector<Vector2> inset;
    inset.reserve(outer.size());
    for (const Vector2& p : outer) {
        inset.push_back({
            center.x + (p.x - center.x) * scale,
            center.y + (p.y - center.y) * scale
        });
    }
    return inset;
}

void AnimationSystem::renderTunnelLine(const TunnelLine& line,
                                       const std::vector<Vector2>& outerShape,
                                       Vector2 center, float maxInset) const {
    if (outerShape.empty()) return;

    float insetRatio   = 1.0f - line.progress;
    auto  currentShape = calculateInsetShape(outerShape, center, insetRatio, maxInset);

    // Lines fade IN as they approach the wall (alpha 0→255)
    unsigned char alpha = static_cast<unsigned char>(line.progress * 255.0f);
    Color c = currentTunnelConfig.lineColor;
    c.a = alpha;

    for (size_t i = 0; i < currentShape.size(); ++i) {
        size_t j = (i + 1) % currentShape.size();
        DrawLineEx(currentShape[i], currentShape[j], tunnelLineThickness, c);
    }
}
