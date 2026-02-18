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

void AnimationSystem::init(int width, int height) {
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
			// For now, leave empty - rendering functions will handle it
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

	// Progress: 0 = inset from edge, 1 = at edge
	float easedProgress = easeInQuad(line.progress);
	
	// Inset ratio: starts at 1.0 (fully inset), ends at 0.0 (at edge)
	float insetRatio = 1.0f - easedProgress;

	// Calculate inset shape maintaining proper polygon offset
	std::vector<Vector2> currentShape = calculateInsetShape(outerShape, center, insetRatio, maxInset);

	// Apply fade-in effect
	unsigned char alpha = static_cast<unsigned char>(line.progress * 255);
	Color fadedColor = currentTunnelConfig.lineColor;
	fadedColor.a = alpha;

	// Draw lines between consecutive points (forming a closed polygon)
	for (size_t i = 0; i < currentShape.size(); i++) {
		size_t nextIndex = (i + 1) % currentShape.size(); // Wrap around to close the shape
		DrawLineEx(currentShape[i], currentShape[nextIndex], tunnelLineThickness, fadedColor);
	}
}

void AnimationSystem::renderTunnelEffect() const {
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
	std::vector<Vector2> outerShape = createRectangularShape(
		borderThickness, borderThickness,
		screenWidth - borderThickness, screenHeight - borderThickness
	);

	for (const auto& line : tunnelLines) {
		renderTunnelLine(line, outerShape, center, contentInset);
	}
}

void AnimationSystem::renderTunnelEffectCustom(int borderLeft, int borderTop, 
												int borderRight, int borderBottom) const {
	if (!tunnelEffectEnabled || tunnelLines.empty()) return;

	int contentInset = currentTunnelConfig.contentInset;

	// Calculate center of custom area
	Vector2 center = {
		static_cast<float>((borderLeft + borderRight) / 2),
		static_cast<float>((borderTop + borderBottom) / 2)
	};

	// Define outer shape (end positions)
	std::vector<Vector2> outerShape = createRectangularShape(
		borderLeft, borderTop, borderRight, borderBottom
	);

	for (const auto& line : tunnelLines) {
		renderTunnelLine(line, outerShape, center, contentInset);
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