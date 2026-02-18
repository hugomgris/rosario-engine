#include "../../incs/AnimationSystem.hpp"

AnimationSystem::AnimationSystem() :
	tunnelLineThickness(2.0),
	tunnelEffectEnabled(false),
	shakeTimer(0.0f),
	shakeOffset({0.0f, 0.0f}),
	screenWidth(1920),
	screenHeight(1080) {
	lastTunnelSpawn = std::chrono::high_resolution_clock::now();
}

void AnimationSystem::init(GameState *gameState, int width, int height) {
	state = gameState;
	screenWidth = width;
	screenHeight = height;
}

void AnimationSystem::enableTunnelEffect(bool enabled, const TunnelConfig &config) {
	tunnelEffectEnabled = enabled;
	currentTunnelConfig = config;
	if (!enabled) {
		clearTunnelEffect();
	}
}

void AnimationSystem::clearTunnelEffect() {
	tunnelLines.clear();
	lastTunnelSpawn = std::chrono::high_resolution_clock::now();
}

void AnimationSystem::updateTunnelEffect(float deltaTime) {
	if (!tunnelEffectEnabled) return;

	// Age existing lines and update their progress
	for (auto& line : tunnelLines) {
		line.age += deltaTime;
		line.progress = easeInQuad(line.age * currentTunnelConfig.animationSpeed);
	}

	// Remove lines that have completed their animation
	tunnelLines.erase(
		std::remove_if(tunnelLines.begin(), tunnelLines.end(),
			[](const TunnelLine& line) { return line.progress >= 1.0f; }),
		tunnelLines.end()
	);

	// Spawn new lines at regular intervals
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = now - lastTunnelSpawn;
	
	if (elapsed.count() >= currentTunnelConfig.spawnInterval) {
		if (static_cast<int>(tunnelLines.size()) < currentTunnelConfig.maxLines) {
			TunnelLine newLine;
			// Points will be set during rendering based on context
			tunnelLines.push_back(newLine);
		}
		lastTunnelSpawn = now;
	}
}

void AnimationSystem::renderTunnelLine(const TunnelLine& line,
                                        const std::vector<Vector2>& outerShape,
                                        const Vector2& center,
                                        float maxInset) const {
    if (outerShape.empty()) return;

    // progress 0 = just spawned = start at inset (near center)
    // progress 1 = dying       = at the outline (outer shape)
    float insetRatio = 1.0f - line.progress;

    std::vector<Vector2> currentShape = calculateInsetShape(outerShape, center, insetRatio, maxInset);

    // Fade OUT as lines reach the wall (alpha goes 255 → 0)
    unsigned char alpha = static_cast<unsigned char>(line.progress * 255);
    Color fadedColor = currentTunnelConfig.lineColor;
    fadedColor.a = alpha;

    for (size_t i = 0; i < currentShape.size(); i++) {
        size_t nextIndex = (i + 1) % currentShape.size();
        DrawLineEx(currentShape[i], currentShape[nextIndex], tunnelLineThickness, fadedColor);
    }
}

void AnimationSystem::renderTunnelEffect() {
	if (!tunnelEffectEnabled || tunnelLines.empty()) return;

	int borderThickness = currentTunnelConfig.borderThickness;
	int contentInset = currentTunnelConfig.contentInset;
	
	int startLeft = borderThickness + contentInset;
	int startTop = borderThickness + contentInset;
	int startRight = screenWidth - borderThickness - contentInset;
	int startBottom = screenHeight - borderThickness - contentInset;

	// Calculate center for inset calculations
	Vector2 center = {
		static_cast<float>(screenWidth / 2),
		static_cast<float>(screenHeight / 2)
	};

	// Define outer shape (end positions)
		tunnelLineShape = createRectangularShape(
		borderThickness, borderThickness,
		screenWidth - borderThickness, screenHeight - borderThickness
	);

	for (const auto& line : tunnelLines) {
		renderTunnelLine(line, tunnelLineShape, center, contentInset);
	}
}

void AnimationSystem::renderTunnelEffectCustom(int borderLeft, int borderTop, 
												int borderRight, int borderBottom) {
	if (!tunnelEffectEnabled || tunnelLines.empty()) return;

	int contentInset = currentTunnelConfig.contentInset;

	// Calculate center of custom area
	Vector2 center = {
		static_cast<float>((borderLeft + borderRight) / 2),
		static_cast<float>((borderTop + borderBottom) / 2)
	};

	tunnelLineShape = state->arena->getArenaOutline(borderLeft, borderTop);

	for (const auto& line : tunnelLines) {
		renderTunnelLine(line, tunnelLineShape, center, contentInset);
	}
}

void AnimationSystem::triggerScreenShake(const ScreenShakeConfig &config) {
	shakeConfig = config;
	shakeTimer = config.duration;
}

void AnimationSystem::updateScreenShake(float deltaTime) {
	if (shakeTimer <= 0.0f) {
		shakeOffset = {0.0f, 0.0f};
		return;
	}

	shakeTimer -= deltaTime;
	
	// Simple random shake within intensity bounds
	float intensity = shakeConfig.intensity * (shakeTimer / shakeConfig.duration);
	shakeOffset.x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * intensity;
	shakeOffset.y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * intensity;
}

// tunnel effec helpers
// mainly used for the default lines in menu (for now, at least)
std::vector<Vector2> AnimationSystem::createRectangularShape(int left, int top, int right, int bottom) const {
	return {
		{static_cast<float>(left), static_cast<float>(top)},		// Top-left
		{static_cast<float>(right), static_cast<float>(top)},		// Top-right
		{static_cast<float>(right), static_cast<float>(bottom)},	// Bottom-right
		{static_cast<float>(left), static_cast<float>(bottom)}		// Bottom-left
	};
}

// inset polygon calculation
std::vector<Vector2> AnimationSystem::calculateInsetShape(const std::vector<Vector2>& outerShape,
                                                           const Vector2& center,
                                                           float insetRatio,
                                                           float maxInsetPixels) const {
    if (outerShape.empty()) return {};

    // Find the average distance from center to outline points
    // This gives us a stable reference to convert pixel inset → scale factor
    float avgDist = 0.0f;
    for (const Vector2& point : outerShape) {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        avgDist += std::sqrt(dx * dx + dy * dy);
    }
    avgDist /= static_cast<float>(outerShape.size());

    if (avgDist < 0.001f) return outerShape;

    // minScale: how far in (as a ratio of avgDist) the inset is allowed to go
    float minScale = std::max(0.0f, (avgDist - maxInsetPixels) / avgDist);

    // Lerp between minScale (spawn) and 1.0 (at outline) using the same scale for ALL points
    float scale = minScale + (1.0f - minScale) * (1.0f - insetRatio);

    std::vector<Vector2> insetShape;
    insetShape.reserve(outerShape.size());

    for (const Vector2& point : outerShape) {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        insetShape.push_back({
            center.x + dx * scale,
            center.y + dy * scale
        });
    }

    return insetShape;
}