#pragma once
#include <vector>
#include <functional>
#include <raylib.h>

// ─── Particle types ───────────────────────────────────────────────────────────

enum class ParticleType {
    Dust,       // Ambient floating squares, no velocity, slow fade
    Explosion,  // Fast outward-flying squares, bright fade
    Trail,      // Snake/menu trail: directional, short lifetime
};

// ─── Particle ─────────────────────────────────────────────────────────────────

struct Particle {
    float x, y;           // screen position
    float vx, vy;         // velocity (pixels/sec)
    float age;            // seconds alive
    float lifetime;       // seconds until dead
    float initialSize;    // starting square half-extent
    float currentSize;    // current square half-extent (shrinks to ~1)
    float rotation;       // degrees
    float rotationSpeed;  // degrees/sec
    ParticleType type;
    Color color;

    // Dust: no velocity, random size/lifetime
    Particle(float px, float py,
             float minSize, float maxSize,
             float minLifetime, float maxLifetime);

    // Explosion / Trail: velocity + custom color
    Particle(float px, float py,
             float minSize, float maxSize,
             float minLifetime, float maxLifetime,
             float velocityX, float velocityY,
             Color particleColor,
             ParticleType ptype = ParticleType::Explosion);
};

// ─── ParticleSystem ───────────────────────────────────────────────────────────
//
// Completely decoupled from ECS. Caller owns it and calls update(dt)/render()
// every frame. Spawn helpers mirror the old OOP version exactly.
//
class ParticleSystem {
public:
    ParticleSystem(int screenW = 1920, int screenH = 1080,
                   int maxDust = 60, float dustInterval = 0.08f);

    void update(float dt);
    void render() const;

    // ── Spawn helpers ─────────────────────────────────────────────────────────
    void spawnExplosion(float x, float y, int count = 20);
    void spawnSnakeTrail(float x, float y, int count, float directionDeg, Color color);
    void spawnDirectedParticles(float x, float y, int count,
                                float directionDeg, float spreadDeg,
                                float minSpeed, float maxSpeed);
    void spawnDirectedParticlesInArea(float cx, float cy, float areaW, float areaH,
                                     int count, float directionDeg, float spreadDeg,
                                     float minSpeed, float maxSpeed, Color color);

private:
    std::vector<Particle> particles;

    int   screenW;
    int   screenH;
    int   maxDustDensity;
    float dustSpawnInterval;
    float dustSpawnTimer;

    // Dust size/lifetime ranges
    float dustMinSize    = 4.0f;
    float dustMaxSize    = 20.0f;
    float dustMinLifetime = 3.0f;
    float dustMaxLifetime = 5.0f;

    // Explosion size range
    float explosionMinSize = 1.0f;
    float explosionMaxSize = 50.0f;

    Color explosionColor = { 255, 200, 60, 255 };

    void spawnDustParticle();
    void drawRotatedSquare(float x, float y, float size,
                           float rotation, Color color, unsigned char alpha) const;
};
