#include "systems/ParticleSystem.hpp"
#include <raymath.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ─── Particle constructors ────────────────────────────────────────────────────

static float frand() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

Particle::Particle(float px, float py,
                   float minSize, float maxSize,
                   float minLifetime, float maxLifetime)
    : x(px), y(py), vx(0.0f), vy(0.0f), age(0.0f),
      type(ParticleType::Dust), color({ 255, 248, 227, 255 }) {
    initialSize   = minSize + frand() * (maxSize - minSize);
    currentSize   = initialSize;
    lifetime      = minLifetime + frand() * (maxLifetime - minLifetime);
    rotation      = frand() * 360.0f;
    rotationSpeed = -30.0f + frand() * 60.0f;
}

Particle::Particle(float px, float py,
                   float minSize, float maxSize,
                   float minLifetime, float maxLifetime,
                   float velocityX, float velocityY,
                   Color particleColor,
                   ParticleType ptype)
    : x(px), y(py), vx(velocityX), vy(velocityY), age(0.0f),
      type(ptype), color(particleColor) {
    initialSize   = minSize + frand() * (maxSize - minSize);
    currentSize   = initialSize;
    lifetime      = minLifetime + frand() * (maxLifetime - minLifetime);
    rotation      = frand() * 360.0f;
    rotationSpeed = -50.0f + frand() * 100.0f;
}

// ─── ParticleSystem ───────────────────────────────────────────────────────────

ParticleSystem::ParticleSystem(int sw, int sh, int maxDust, float dustInterval)
    : screenW(sw), screenH(sh),
      maxDustDensity(maxDust),
      dustSpawnInterval(dustInterval),
      dustSpawnTimer(0.0f) {
    particles.reserve(maxDustDensity);
}

void ParticleSystem::update(float dt) {
    // Ambient dust spawning
    dustSpawnTimer += dt;
    if (dustSpawnTimer >= dustSpawnInterval) {
        spawnDustParticle();
        dustSpawnTimer = 0.0f;
    }

    // Integrate all particles
    for (auto& p : particles) {
        p.age       += dt;
        p.rotation  += p.rotationSpeed * dt;
        p.x         += p.vx * dt;
        p.y         += p.vy * dt;

        float progress  = p.age / p.lifetime;
        p.currentSize   = p.initialSize * (1.0f - progress) + 1.0f * progress;
    }

    // Prune dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return p.age >= p.lifetime; }),
        particles.end()
    );
}

void ParticleSystem::render() const {
    for (const auto& p : particles) {
        float progress    = p.age / p.lifetime;
        unsigned char alpha;
        if (p.type == ParticleType::Dust)
            alpha = static_cast<unsigned char>((1.0f - progress) * 200);
        else
            alpha = static_cast<unsigned char>((1.0f - progress) * 255);

        drawRotatedSquare(p.x, p.y, p.currentSize, p.rotation, p.color, alpha);
    }
}

// ─── Spawn helpers ────────────────────────────────────────────────────────────

void ParticleSystem::spawnDustParticle() {
    int dustCount = 0;
    for (const auto& p : particles)
        if (p.type == ParticleType::Dust) ++dustCount;
    if (dustCount >= maxDustDensity) return;

    float x = frand() * static_cast<float>(screenW);
    float y = frand() * static_cast<float>(screenH);
    particles.emplace_back(x, y, dustMinSize, dustMaxSize, dustMinLifetime, dustMaxLifetime);
}

void ParticleSystem::spawnExplosion(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = frand() * 360.0f * DEG2RAD;
        float speed = 50.0f + frand() * 150.0f;
        particles.emplace_back(x, y,
            explosionMinSize, explosionMaxSize, 0.5f, 1.0f,
            cosf(angle) * speed, sinf(angle) * speed,
            explosionColor, ParticleType::Explosion);
    }
}

void ParticleSystem::spawnSnakeTrail(float x, float y, int count,
                                     float directionDeg, Color color) {
    float angle = directionDeg * DEG2RAD;
    for (int i = 0; i < count; ++i) {
        float ox    = frand() * 30.0f;
        float oy    = frand() * 30.0f;
        float speed = 50.0f + frand() * 9.0f;
        float life  = 0.2f + frand() * 1.5f;

        Particle p(x + ox, y + oy, 10.0f, 20.0f, life, life,
                   cosf(angle) * speed, sinf(angle) * speed, color, ParticleType::Trail);
        p.rotation      = 0.0f;
        p.rotationSpeed = 0.0f;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnDirectedParticles(float x, float y, int count,
                                            float directionDeg, float spreadDeg,
                                            float minSpeed, float maxSpeed) {
    float baseAngle  = directionDeg * DEG2RAD;
    float spreadRad  = spreadDeg   * DEG2RAD;

    for (int i = 0; i < count; ++i) {
        float angle = baseAngle + (frand() - 0.5f) * spreadRad;
        float speed = minSpeed  + frand() * (maxSpeed - minSpeed);
        Color c = { 70, 130, 180, 255 };
        particles.emplace_back(x, y, 5.0f, 15.0f, 1.0f, 2.0f,
                               cosf(angle) * speed, sinf(angle) * speed, c, ParticleType::Explosion);
    }
}

void ParticleSystem::spawnDirectedParticlesInArea(float cx, float cy,
                                                  float areaW, float areaH,
                                                  int count,
                                                  float directionDeg, float spreadDeg,
                                                  float minSpeed, float maxSpeed,
                                                  Color color) {
    float baseAngle = directionDeg * DEG2RAD;
    float spreadRad = spreadDeg   * DEG2RAD;

    for (int i = 0; i < count; ++i) {
        float spawnX = cx + (frand() - 0.5f) * areaW;
        float spawnY = cy + (frand() - 0.5f) * areaH;
        float angle  = baseAngle + (frand() - 0.5f) * spreadRad;
        float speed  = minSpeed  + frand() * (maxSpeed - minSpeed);
        particles.emplace_back(spawnX, spawnY, 5.0f, 15.0f, 1.0f, 2.0f,
                               cosf(angle) * speed, sinf(angle) * speed, color, ParticleType::Explosion);
    }
}

// ─── Draw helper ─────────────────────────────────────────────────────────────

void ParticleSystem::drawRotatedSquare(float x, float y, float size,
                                       float rotation, Color color,
                                       unsigned char alpha) const {
    Color c = { color.r, color.g, color.b, alpha };
    Rectangle rect   = { x, y, size, size };
    Vector2   origin = { size / 2.0f, size / 2.0f };
    DrawRectanglePro(rect, origin, rotation, c);
}
