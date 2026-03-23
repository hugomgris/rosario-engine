#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <raylib.h>
#include "../animations/ParticleConfig.hpp"
#include "../ecs/Registry.hpp"
#include "../components/ParticleSpawnRequest.hpp"
#include "../../incs/FrameContext.hpp"

using pType = ParticleSpawnRequest::ParticleType;

struct Particle {
	float   x, y;
	float   vx, vy;
	float   rotation, rotationSpeed;
	float   initialSize, currentSize;
	float   lifetime, age;
	pType	type;
	Color   color;

	Particle(float px, float py,
			float minSize, float maxSize,
			float minLifetime, float maxLifetime,
			Color c,
			pType t,
			float velocityX = 0.0f,
			float velocityY = 0.0f);
};

class ParticleSystem {
	std::vector<Particle>   _particles;

	int     _screenWidth;
	int     _screenHeight;
	float   _dustSpawnTimer  = 0.0f;
	float   _trailSpawnTimer = 0.0f;
	std::unordered_map<unsigned int, float> _menuTrailEmitterTimers;
	size_t  _lastMenuTrailRequestCount = 0;
	ParticleConfig          _config;

	// internal spawn helpers
	void    spawnDust(const ArenaBounds& bounds);
	void    spawnExplosion(float x, float y);
	void    spawnTrail(float x, float y, Direction direction, Color color);
	void    spawnMenuTrail(float x, float y, Direction direction, Color color, int count);

	// render helper
	void    drawRotatedSquare(float cx, float cy, float size,
							float rotation, Color color, unsigned char alpha) const;

public:
	ParticleSystem(int screenW, int screenH, ParticleConfig config);

	void    update(float dt, Registry& registry, const FrameContext& ctx);   // consumes ParticleSpawnRequests
	void    render() const;

	// direct spawn compatibility helpers
	void    spawnTrailAt(float x, float y, Color color);
	void    spawnMenuTrailAt(float x, float y, Color color);

	// selective clears
	void    clear();            // clears everything
	void    clearGameplay();    // clears Trail + Explosion + Dust (keeps MenuTrail)
	void    clearMenuTrail();   // clears only MenuTrail
	void    handleStateTransition(GameState previousState, GameState currentState);
	size_t  getParticleCount() const;
	size_t  getMenuTrailParticleCount() const;
	size_t  getLastMenuTrailRequestCount() const;
};