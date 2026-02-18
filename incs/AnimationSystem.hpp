#pragma once

#include <raylib.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>

struct TunnelLine {
	float progress;        // 0 = at center, 1 = at edge
	float age;

	std::vector<Vector2> points;
	
	TunnelLine() : progress(0.0f), age(0.0f) {
		
	}
};

struct TunnelConfig {
	int     borderThickness;
	int     contentInset;
	float   spawnInterval;
	float   animationSpeed;
	int     maxLines;
	Color   lineColor;

	// menu default
	static TunnelConfig menu() {
		return {20, 80, 0.3f, 0.5f, 8, Color{70, 130, 180, 255}};
	}

	// 2d realm
	static TunnelConfig realm2D() {
		return {15, 40, 0.15f, 0.7f, 12, Color{144, 238, 144, 255}};
	}
};

// screen shake effect, experimenting
struct ScreenShakeConfig {
	float intensity;
	float duration;
	float frequency;
};

// system
class AnimationSystem {
	private:
		std::vector<TunnelLine>                         tunnelLines;
		std::chrono::high_resolution_clock::time_point  lastTunnelSpawn;
		TunnelConfig                                    currentTunnelConfig;
		ScreenShakeConfig                               shakeConfig;
		bool                                            tunnelEffectEnabled;
		float											tunnelLineThickness;

		// currently unused
		float   shakeTimer;
		Vector2 shakeOffset;

		// screen dimensions
		int screenWidth;
		int screenHeight;

		// Easing helpers
		float easeInQuad(float t) const {
			return t * t;
		}

		float easeOutQuad(float t) const {
			return t * (2 - t);
		}

		float easeInOutCubic(float t) const {
			return t < 0.5f ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
		}

		// TRANSITION TO POINT BASED TUNNEL LINE - HELPERS
		// Helper to create rectangular shape (current behavior)
		std::vector<Vector2> createRectangularShape(int left, int top, int right, int bottom) const {
			return {
				{static_cast<float>(left), static_cast<float>(top)},      // Top-left
				{static_cast<float>(left + 500), static_cast<float>(top)},      // Top-left
				{static_cast<float>(left + 500), static_cast<float>(top + 500)},      // Top-left
				{static_cast<float>(left + 1500), static_cast<float>(top + 500)},      // Top-left
				{static_cast<float>(left + 1500), static_cast<float>(top)},      // Top-left
				{static_cast<float>(right), static_cast<float>(top)},     // Top-right
				{static_cast<float>(right), static_cast<float>(top + 300)},     // Top-right
				{static_cast<float>(right - 200), static_cast<float>(top) + 300},     // Top-right
				{static_cast<float>(right - 200), static_cast<float>(top) + 600},     // Top-right
				{static_cast<float>(right), static_cast<float>(top + 600)},     // Top-right
				{static_cast<float>(right), static_cast<float>(bottom)},  // Bottom-right
				{static_cast<float>(left), static_cast<float>(bottom)}    // Bottom-left
			};
		}
		
		// Helper to calculate inset for entire polygon shape
		// Returns a new polygon inset from the outer shape using proper miter calculation
		std::vector<Vector2> calculateInsetShape(const std::vector<Vector2>& outerShape,
		                                         const Vector2& center,
		                                         float insetRatio, 
		                                         float maxInsetPixels) const {
			if (outerShape.empty()) return {};
			
			std::vector<Vector2> insetShape;
			insetShape.reserve(outerShape.size());
			
			float actualInset = insetRatio * maxInsetPixels;
			
			for (size_t i = 0; i < outerShape.size(); i++) {
				const Vector2& current = outerShape[i];
				const Vector2& prev = outerShape[(i == 0) ? outerShape.size() - 1 : i - 1];
				const Vector2& next = outerShape[(i + 1) % outerShape.size()];
				
				// Calculate edge vectors (from prev to current, from current to next)
				Vector2 edge1 = {current.x - prev.x, current.y - prev.y};
				Vector2 edge2 = {next.x - current.x, next.y - current.y};
				
				// Normalize edge vectors
				float len1 = std::sqrt(edge1.x * edge1.x + edge1.y * edge1.y);
				float len2 = std::sqrt(edge2.x * edge2.x + edge2.y * edge2.y);
				
				if (len1 > 0.001f) {
					edge1.x /= len1;
					edge1.y /= len1;
				}
				if (len2 > 0.001f) {
					edge2.x /= len2;
					edge2.y /= len2;
				}
				
				// Calculate perpendicular inward normals for each edge
				// For a CCW polygon, rotating edge 90° CCW gives outward normal
				// So we rotate 90° CW (or -90° CCW) to get inward normal
				Vector2 normal1 = {edge1.y, -edge1.x};  // 90° CW rotation
				Vector2 normal2 = {edge2.y, -edge2.x};
				
				// Verify they point inward (toward center)
				Vector2 toCenter = {center.x - current.x, center.y - current.y};
				float dot1 = normal1.x * toCenter.x + normal1.y * toCenter.y;
				float dot2 = normal2.x * toCenter.x + normal2.y * toCenter.y;
				
				// If normals point outward, flip them
				if (dot1 < 0) {
					normal1.x = -normal1.x;
					normal1.y = -normal1.y;
				}
				if (dot2 < 0) {
					normal2.x = -normal2.x;
					normal2.y = -normal2.y;
				}
				
				// Calculate miter vector (bisector of the two normals)
				Vector2 miter = {
					normal1.x + normal2.x,
					normal1.y + normal2.y
				};
				
				float miterLen = std::sqrt(miter.x * miter.x + miter.y * miter.y);
				if (miterLen > 0.001f) {
					miter.x /= miterLen;
					miter.y /= miterLen;
				}
				
				// Calculate miter length
				// The miter length is: inset / dot(miter, normal)
				// This ensures uniform perpendicular distance from edges
				float miterDot = miter.x * normal1.x + miter.y * normal1.y;
				float miterScale = 1.0f;
				
				if (std::abs(miterDot) > 0.001f) {
					miterScale = 1.0f / miterDot;
				}
				
				// Clamp extreme miter lengths (for very acute angles)
				if (miterScale > 10.0f) miterScale = 10.0f;
				if (miterScale < 0.1f) miterScale = 0.1f;
				
				// Apply inset with proper miter scaling
				Vector2 insetPoint = {
					current.x + miter.x * actualInset * miterScale,
					current.y + miter.y * actualInset * miterScale
				};
				
				insetShape.push_back(insetPoint);
			}
			
			return insetShape;
		}

		void renderTunnelLine(const TunnelLine& line, 
                          const std::vector<Vector2>& outerShape,
                          const Vector2& center,
                          float maxInset) const;

	public:
		AnimationSystem();
		~AnimationSystem() = default;

		void init(int width, int heigth);

		// tunnel effect managers
		void enableTunnelEffect(bool enabled, const TunnelConfig &config = TunnelConfig::menu());
		void updateTunnelEffect(float deltaTime);
		void renderTunnelEffect() const;
		void renderTunnelEffectCustom(int left, int top, int right, int bottom) const;  // Custom border area
		void clearTunnelEffect();		// tunnel effect getters
		const std::vector<TunnelLine>& getTunnelLines() const { return tunnelLines; }
		const TunnelConfig& getTunnelConfig() const { return currentTunnelConfig; }
		bool isTunnelEffectEnabled() const { return tunnelEffectEnabled; }

		// scren shake managers (unsued for now)
		void triggerScreenShake(const ScreenShakeConfig &config);
		void updateScreenShake(float deltaTime);
		Vector2 getScreenShakeOffset() const { return shakeOffset; }

		// fade effect managers (future, placeholdedr for now)
		// void startFade(float duration, Color fromColor, Color toColor);
		// void updateFade(float dt);
		// void renderFade() const;
};