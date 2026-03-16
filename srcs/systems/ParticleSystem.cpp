#include "../../incs/DataStructs.hpp"
#include "ParticleSystem.hpp"
#include "../../incs/RaylibColors.hpp"
#include <raylib.h>
#include <raymath.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float randFloat(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

static float directionToAngle(Direction dir) {
    switch (dir) {
        case Direction::RIGHT:  return 0.0f;
        case Direction::DOWN:   return PI / 2.0f;
        case Direction::LEFT:   return PI;
        case Direction::UP:     return 3.0f * PI / 2.0f;
    }
    return 0.0f;
}

Particle::Particle(float px, float py,
                   float minSize, float maxSize,
                   float minLifetime, float maxLifetime,
                   Color c,
                   pType t,
                   float velocityX,
                   float velocityY)
    : x(px), y(py),
      vx(velocityX), vy(velocityY),
      rotation(randFloat(0.0f, 360.0f)),
      rotationSpeed(randFloat(-120.0f, 120.0f)),
      initialSize(randFloat(minSize, maxSize)),
      currentSize(initialSize),
      lifetime(randFloat(minLifetime, maxLifetime)),
      age(0.0f),
      type(t),
      color(c)
{}

ParticleSystem::ParticleSystem(int screenW, int screenH, ParticleConfig config)
    : _screenWidth(screenW), _screenHeight(screenH), _config(config)
{
    _particles.reserve(512);
}

void ParticleSystem::spawnDust(const ArenaBounds& bounds) {
    int dustCount = 0;
    for (const auto& p : _particles)
        if (p.type == pType::Dust) dustCount++;
    if (dustCount >= _config.dustMaxDensity)
        return;

    float x = bounds.x + randFloat(0.0f, bounds.width);
    float y = bounds.y + randFloat(0.0f, bounds.height);

    _particles.emplace_back(
        x, y,
        _config.dustMinSize,    _config.dustMaxSize,
        _config.dustMinLifetime, _config.dustMaxLifetime,
        customGray,
        pType::Dust
        // vx, vy default to 0
    );
}

void ParticleSystem::spawnExplosion(float x, float y) {
    for (int i = 0; i < _config.explosionCount; ++i) {
        float angle = randFloat(0.0f, 2.0f * PI);
        float speed = randFloat(_config.explosionMinSpeed, _config.explosionMaxSpeed);

        _particles.emplace_back(
            x, y,
            _config.explosionMinSize,    _config.explosionMaxSize,
            _config.explosionMinLifetime, _config.explosionMaxLifetime,
            food2DColor, 
            pType::Explosion,
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
    }
}

void ParticleSystem::spawnTrailAt(float x, float y, Color color) {
    spawnTrail(x, y, Direction::RIGHT, color); // direction ignored: scatter only
}

void ParticleSystem::spawnMenuTrailAt(float x, float y, Color color) {
    float scatter = static_cast<float>(_config.menuTrailScatter);
    for (int i = 0; i < _config.menuTrailCount; ++i) {
        float spawnX = x + randFloat(-scatter * 0.3f, scatter * 0.3f);
        float spawnY = y + randFloat(-scatter,         scatter);
        // Emit rightward: base angle 0 (positive X), spread ±PI/4 (45 degrees)
        float angle  = randFloat(-PI / 4.0f, PI / 4.0f);
        float speed  = randFloat(_config.menuTrailMinSpeed, _config.menuTrailMaxSpeed);
        Particle p(
            spawnX, spawnY,
            _config.menuTrailMinSize, _config.menuTrailMaxSize,
            _config.menuTrailMinLifetime, _config.menuTrailMaxLifetime,
            color,
            pType::MenuTrail,
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
        _particles.push_back(p);
    }
}

void ParticleSystem::spawnTrail(float x, float y, Direction direction, Color color) {
    float baseAngle = directionToAngle(direction);
    float scatter   = static_cast<float>(_config.trailCount > 1 ? _config.trailScatter : 0);

    for (int i = 0; i < _config.trailCount; ++i) {
        float spawnX = x + randFloat(-scatter, scatter);
        float spawnY = y + randFloat(-scatter, scatter);

        float spread = randFloat(-0.4f, 0.4f);
        float angle  = baseAngle + spread + PI; // emit backwards
        float speed  = randFloat(_config.trailMinSpeed, _config.trailMaxSpeed);

        Particle p(
            spawnX, spawnY,
            _config.trailMinSize, _config.trailMaxSize,
            _config.trailMinLifetime, _config.trailMaxLifetime,
            color,
            pType::Trail,
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
        p.rotationSpeed = 0.0f;
        p.rotation      = 0.0f;
        _particles.push_back(p);
    }
}

void ParticleSystem::drawRotatedSquare(float cx, float cy, float size,
                                       float rotation, Color color,
                                       unsigned char alpha) const {
    Color c     = color;
    c.a         = alpha;
    Rectangle r = { cx, cy, size, size };
    Vector2 origin = { size * 0.5f, size * 0.5f };
    DrawRectanglePro(r, origin, rotation, c);
}

void ParticleSystem::update(float dt, Registry& registry, const FrameContext& ctx) {
    // 1. consume ParticleSpawnRequests from the registry
    // advance trail timer
    bool emitTrail = (_config.trailSpawnInterval <= 0.0f);
    if (_config.trailSpawnInterval > 0.0f) {
        _trailSpawnTimer += dt;
        if (_trailSpawnTimer >= _config.trailSpawnInterval) {
            _trailSpawnTimer = 0.0f;
            emitTrail = true;
        }
    }

    registry.forEach<ParticleSpawnRequest>([&](Entity e, ParticleSpawnRequest& req) {
        float spawnX = req.x;
        float spawnY = req.y;

        if (req.gridCoords && ctx.cellSize > 0) {
            spawnX = ctx.gameAreaX + req.x * ctx.cellSize + ctx.cellSize * 0.5f;
            spawnY = ctx.gameAreaY + req.y * ctx.cellSize + ctx.cellSize * 0.5f;
        }

        switch (req.type) {
            case pType::Explosion:
                spawnExplosion(spawnX, spawnY);
                break;
            case pType::Trail:
                if (emitTrail)
                    spawnTrail(spawnX, spawnY, req.direction, req.color);
                break;
            case pType::Dust:
                break;
            case pType::MenuTrail:
                // MenuTrail particles are spawned directly via spawnMenuTrailAt(),
                // not through the spawn-request queue — nothing to do here.
                break;
        }
        registry.removeComponent<ParticleSpawnRequest>(e);
    });

    // 2. ambient dust spawning
    _dustSpawnTimer += dt;
    if (_dustSpawnTimer >= _config.dustSpawnInterval) {
        _dustSpawnTimer = 0.0f;
        const float cs = static_cast<float>(ctx.cellSize);
        ArenaBounds playfield {
            ctx.gameAreaX + cs,
            ctx.gameAreaY + cs,
            static_cast<float>(ctx.gridWidth  - 2) * cs,
            static_cast<float>(ctx.gridHeight - 2) * cs
        };
        spawnDust(playfield);
    }

    // 3. simulate all particles
    for (auto& p : _particles) {
        p.age += dt;

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        float drag = (p.type == pType::Dust)      ? 0.98f
                   : (p.type == pType::MenuTrail)  ? 0.97f
                   : 0.92f;
        p.vx *= drag;
        p.vy *= drag;

        p.rotation += p.rotationSpeed * dt;

        float lifeRatio = 1.0f - (p.age / p.lifetime);
        p.currentSize   = p.initialSize * lifeRatio;
    }

    // 4. remove dead particles
    _particles.erase(
        std::remove_if(_particles.begin(), _particles.end(),
            [](const Particle& p) { return p.age >= p.lifetime; }),
        _particles.end()
    );
}

void ParticleSystem::render() const {
    for (const auto& p : _particles) {
        float lifeRatio = 1.0f - (p.age / p.lifetime);
        unsigned char alpha = (unsigned char)(lifeRatio * 255.0f);

        drawRotatedSquare(p.x, p.y, p.currentSize, p.rotation, p.color, alpha);
    }
}

void ParticleSystem::clear() {
    _particles.clear();
}

void ParticleSystem::clearGameplay() {
    _particles.erase(
        std::remove_if(_particles.begin(), _particles.end(),
            [](const Particle& p) { return p.type != pType::MenuTrail; }),
        _particles.end());
}

void ParticleSystem::clearMenuTrail() {
    _particles.erase(
        std::remove_if(_particles.begin(), _particles.end(),
            [](const Particle& p) { return p.type == pType::MenuTrail; }),
        _particles.end());
}

size_t ParticleSystem::getParticleCount() const {
    return _particles.size();
}