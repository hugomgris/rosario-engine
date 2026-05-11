# Pong — Port Design Document

> Porting a TypeScript/Pixi.js Pong game to the Rosario C++ engine. This document covers the full design of the port: what maps directly, what needs to be built, and how each mechanic is implemented.

---

## Source Game Summary

The original TypeScript Pong was a full arcade game with:
- Local and online multiplayer
- Classic and extended game modes
- Named arena presets with animated polygon wall shapes ("The Fangs", "The Bowtie", "The Steps")
- Power-ups, power-downs, and ball-type changes (collected mid-game)
- Spawning mid-arena obstacles
- Full menu system with glossary, options, and info screens
- CRT post-processing and particle effects
- Tournament management (online-only, not porting)

**What ports:** All local gameplay, all arena shapes, all power-up mechanics, all ball types, menus, visuals.

**What doesn't port:** Online multiplayer, tournament system, browser detection, i18n, auth/session management. These were web-platform features.

---

## Architecture Mapping

### TypeScript ECS → Rosario ECS

The TS ECS used a "fat entity" model: entities stored their own components in a `Map<string, Component>`, supported multiple components of the same type via `type:instanceId` keys, and had rendering knowledge built in (`getAllRenderables()`).

Rosario's ECS is external storage: components live in typed pools in the Registry, entities are dumb IDs. This is architecturally cleaner and more performant. The port uses Rosario's ECS natively — no attempt to replicate the TS component key system.

| TS Concept | Rosario Equivalent |
|---|---|
| `Entity` with component `Map` | `Entity` ID + `Registry` component pools |
| `component.type` string key | `std::type_index` (automatic, via template) |
| Multiple components of same type | Separate component types (e.g. `BallPhysicsComponent` vs `PhysicsComponent`) |
| `getAllRenderables()` | `RenderSystem` queries `view<RenderComponent>()` |
| `entity.layer` | Render order handled by draw call sequence |
| `UISystem` + Pixi `Container` | `PixelTextRenderSystem` + `UIInteractionSystem` |

### TS Physics → Rosario PhysicsSystem

`PhysicsComponent.ts` contained: x, y, width, height, velocityX, velocityY, isStatic, behaviour (bounce/block/trigger/none), restitution, mass, speed, polygonal flag.

The Rosario `PhysicsComponent` mirrors this closely but lives in the engine, not the game:

```cpp
// engine/components/PhysicsComponent.hpp
struct PhysicsComponent {
    float x, y;              // continuous-space position (NOT grid)
    float width, height;     // AABB extents
    float vx, vy;            // velocity (pixels/sec)
    float restitution;       // bounce coefficient [0..1]
    float mass;
    float speed;             // reference speed for this entity
    bool  isStatic;          // paddles are semi-static (player-driven), walls are fully static
    PhysicsBehaviour behaviour; // Bounce | Block | Trigger | None
};
```

The polygonal collision from TS (for non-rectangular wall shapes) is handled differently in Rosario: complex arena wall shapes are defined as `PongArena` collision surfaces (line segments), not as polygon physics components.

---

## New Components

### `PhysicsComponent` (engine-level)
See Architecture Reference. Used by ball, paddles, obstacles, power-ups.

### `TransformComponent` (engine-level)
```cpp
struct TransformComponent {
    float x, y;        // continuous-space position (mirror of PhysicsComponent for render)
    float rotation;    // degrees, for spinning ball visual
    float scaleX = 1.f;
    float scaleY = 1.f;
};
```

### `BallComponent` (pong-game-level)
```cpp
enum class BallType { Normal, Egg, Square, Arrow, Multi };

struct BallComponent {
    BallType    type        = BallType::Normal;
    float       speedRamp   = 0.f;    // accumulated speed increase per hit
    float       maxSpeed    = 800.f;
    bool        isDecoy     = false;  // decoy balls don't score
    int         ownerSide   = -1;     // -1 = neutral, 0 = left, 1 = right
    float       curvature   = 0.f;    // for Egg ball lateral acceleration
};
```

### `PaddleComponent` (pong-game-level)
```cpp
struct PaddleComponent {
    int     playerSide;       // 0 = left, 1 = right
    float   sizeMultiplier = 1.f;   // power-up size modifier
    bool    controlsInverted = false;
    bool    attractsBalls    = false;
    bool    hasShield        = false;
    bool    canShoot         = false;
    float   affectationTimer = 0.f;  // time remaining on active power effect
};
```

### `PowerUpComponent` (pong-game-level)
```cpp
enum class PowerUpType {
    // Power-ups (positive)
    DoublePaddleSize,
    AttractBalls,
    Shield,
    Shooter,
    // Power-downs (negative)
    HalfPaddleSize,
    InvertControls,
    FlattenReturns,
    SlowPaddle,
    // Ball changes
    EggBall,
    SquareBall,
    ArrowBall,
    MultiBall
};

struct PowerUpComponent {
    PowerUpType type;
    int         targetSide;   // which player this affects when collected
    float       duration;     // seconds the effect lasts (0 = permanent until next reset)
};
```

### `ObstacleComponent` (pong-game-level)
```cpp
struct ObstacleComponent {
    bool isActive = true;
    // ObstacleComponent + PhysicsComponent(isStatic=true) = a solid mid-arena blocker
};
```

### `ShieldComponent` (pong-game-level)
```cpp
struct ShieldComponent {
    int     ownerSide;
    float   x, y, width, height;  // shield AABB, positioned behind paddle
};
```

### `ProjectileComponent` (pong-game-level)
```cpp
struct ProjectileComponent {
    int     ownerSide;
    float   stunDuration = 1.5f;  // how long the paddle is stunned on hit
};
```

---

## New Systems

### `PhysicsSystem` (engine-level)
Continuous-space AABB physics. Runs on all entities with `PhysicsComponent`.

**Responsibilities:**
- Integrate velocity: `pos += vel * dt`
- Clamp static entities (walls, obstacles) — skip integration
- Arena boundary collision: ball bounces off top/bottom walls, goes out-of-bounds on left/right (score event)
- AABB vs. AABB collision detection between dynamic and static entities
- Bounce response: reflect velocity on collision axis, apply restitution

**Ball-paddle deflection (the core feel mechanic):**
```
contactY = (ball.centerY - paddle.centerY) / (paddle.height / 2)
// contactY in [-1, 1] where 0 = center hit
deflectionAngle = contactY * MAX_BOUNCE_ANGLE  // e.g. 75 degrees
ball.vx = cos(deflectionAngle) * ball.speed * sign(ball.vx) * -1
ball.vy = sin(deflectionAngle) * ball.speed
```

This is the difference between Pong that feels good and Pong that doesn't. Center hits are nearly horizontal; edge hits are steep.

**Egg ball curve:**
```
// Applied each frame in PhysicsSystem for BallType::Egg
ball.vy += ball.curvature * dt
```

**Square ball:**
Visual rotation handled by `TransformComponent::rotation += rotationSpeed * dt`. Hitbox remains AABB — the approximation is invisible in practice.

### `PaddleSystem` (pong-game-level)
Reads `InputComponent`, applies continuous movement to `PhysicsComponent::vy`. Unlike snake's buffered input, paddles use `IsKeyDown()` (hold to move), not `IsKeyPressed()` (tap events). The `InputSystem` needs a continuous-poll mode or paddles bypass it and poll Raylib directly — the simpler option initially.

**Responsibilities:**
- Map held keys → paddle velocity
- Clamp paddle to arena bounds
- Apply `sizeMultiplier` to paddle height on change
- Apply `controlsInverted` flag (flip up/down)
- Apply `SlowPaddle` speed modifier from active power-down

### `BallSystem` (pong-game-level)
**Responsibilities:**
- Ball launch on round start (random angle within ±45° of horizontal, alternating serve side)
- Speed ramp: after each paddle hit, `ball.speed += speedRamp`
- Out-of-bounds detection (left/right edges): emit score event, reset ball
- Multi-ball: when `MultiBall` power-up is collected, spawn N additional ball entities; decoys flagged
- Ball type transitions: replace `BallComponent::type`, adjust visual render component

### `PongCollisionSystem` (pong-game-level)
AABB-based, not grid-based. Detects overlaps between dynamic entities (ball) and static entities (paddles, walls, obstacles, shields, power-ups).

Uses the same `CollisionEffectDispatcher` pattern as snake:
```
PongCollisionRules.json → PongCollisionEffects.cpp registered functions
```

Effect examples:
- `Ball × Paddle` → `effectBallPaddleDeflect`
- `Ball × Wall` → `effectBallWallBounce`
- `Ball × PowerUp` → `effectCollectPowerUp`
- `Ball × Shield` → `effectBallShieldBounce` (shield absorbs, then disappears)
- `Ball × Obstacle` → `effectBallObstacleBounce`
- `Projectile × Paddle` → `effectStunPaddle`

### `PowerUpSystem` (pong-game-level)
**Responsibilities:**
- Spawn power-up entities at random positions in the arena on a timer
- Apply effect on collection (modify target `PaddleComponent` fields)
- Track `affectationTimer` on affected paddle; revert effect on expiry
- Visual: affectation bar rendered on the paddle's side of screen (like the TS version)
- Coordinate with `BallSystem` for ball-change power-ups (spawn new ball entity, remove current)

---

## PongArena

Replaces `ArenaGrid` for continuous space. Wall shapes are defined as collections of line segments (or AABB rects) that the physics system tests balls against.

```cpp
// games/pong/arena/PongArena.hpp
struct WallSegment {
    float x1, y1, x2, y2;   // line segment endpoints
    // or for AABB obstacles:
    float x, y, width, height;
    bool  isAABB;
};

class PongArena {
    std::vector<WallSegment>         _walls;
    std::vector<std::vector<Vector2>> _outlines; // for AnimationSystem
    std::string                       _name;
    bool                              _isSpawning, _isDespawning;
    // spawn/despawn animation identical to ArenaGrid pattern
public:
    void loadPreset(const PongArenaPreset&);
    bool ballOutOfBounds(float x, float y) const;
    const std::vector<WallSegment>& getWalls() const;
    std::vector<std::vector<Vector2>> getAllOutlines() const; // feeds AnimationSystem
};
```

### Arena Presets (JSON)

```json
{
  "presets": [
    {
      "name": "Classic",
      "walls": [
        { "type": "rect", "x": 0, "y": 0, "width": 1920, "height": 20 },
        { "type": "rect", "x": 0, "y": 1060, "width": 1920, "height": 20 }
      ],
      "outline": [[0,0],[1920,0],[1920,1080],[0,1080]]
    },
    {
      "name": "The Fangs",
      "walls": [ ... ],
      "outline": [ ... ]
    }
  ]
}
```

The `AnimationSystem` receives the outline polygon list and renders the tunnel effect exactly as it does for snake — no changes needed to the engine system.

---

## Ball Types — Implementation Detail

| Ball Type | Physics Change | Visual Change | Complexity |
|---|---|---|---|
| Normal | None | Yellow circle | — |
| Egg | `curvature` lateral acceleration applied per frame | Ellipse, slight stretch | Low |
| Square | Rotation applied per frame (visual only, AABB hitbox) | Rotating square | Low |
| Arrow | 2× speed, no speed ramp, thinner AABB | Arrow/diamond shape | Low |
| Multi | Spawn N balls, some flagged `isDecoy` | Same as Normal | Medium |

---

## Arena Shapes — Implementation Detail

Each named arena is a preset of wall segments + an outline polygon. The wall segments feed the physics collision system. The outline polygon feeds the `AnimationSystem`.

| Arena | Description | Physics Notes |
|---|---|---|
| Classic | Two horizontal walls | Standard AABB top/bottom |
| The Steps | Stepped walls, multiple levels | Multiple AABB rects |
| The Bowtie | Converging walls, narrow center | Diagonal-ish segments, approximate with AABB |
| The Fangs | Inward-pointing wall protrusions | AABB rects arranged as fangs |

For the first implementation: all shapes approximated as AABB rects. Diagonal walls (true line segment collision) added in a later pass if desired.

---

## Game States

```
Menu
  ↓ (START selected)
Playing
  ↓ (score reaches limit, or time limit)
GameOver
  ↓ (RETRY or MENU selected)
Menu / Playing

Any state → Paused (P key)
Paused → Playing (P key again)
Any state → Exiting (ESC or QUIT)
```

This maps directly onto the existing `GameState` enum. No new states needed.

---

## Power-up Visual Design

Following the TS original's Glossary screen (image 4 in reference screenshots):

**Power-ups (positive, collected by either player):**
- Double Paddle Size — square icon, grid pattern
- Attract Balls — circle/target icon
- Shield — rectangle behind paddle
- Shooter — diamond icon, spawns 3 projectiles

**Power-downs (negative, affects the collecting player):**
- Half Paddle Size
- Invert Controls
- Flatten Returns (reduces angle variation on paddle hits)
- Slow Paddle

**Ball Changes (affects the ball itself):**
- Egg Ball — curved trajectory
- Square Ball — spinning square
- Arrow Ball — fast burst
- Multi Ball — multiple small balls with decoys

Power-up entities rendered as small icons using `RenderComponent` + simple `DrawRectangle`/`DrawCircle` calls initially, upgraded to glyph-based icons later.

---

## Pong main.cpp Structure

Following the same orchestrator pattern as snake's `main.cpp`:

```cpp
int main() {
    // 1. Load configs
    PongArenaPresets arenaPresets = PongArenaPresetLoader::load("data/pong/ArenaPresets.json");
    ParticleConfig   particles    = ParticleConfigLoader::load("data/pong/ParticleConfig.json");
    PostProcessConfig ppConfig    = PostProcessConfigLoader::load("data/pong/PostProcessConfig.json");
    // ... etc

    // 2. Init engine systems
    Registry              registry;
    InputSystem           inputSystem;
    PhysicsSystem         physicsSystem;
    RenderSystem          renderSystem;
    ParticleSystem        particleSystem(SCREEN_W, SCREEN_H, particles);
    AnimationSystem       animationSystem;
    PostProcessingSystem  ppSystem;
    UIInteractionSystem   uiInteractionSystem;

    // 3. Init pong-specific systems
    PongCollisionSystem   collisionSystem;
    PaddleSystem          paddleSystem;
    BallSystem            ballSystem;
    PowerUpSystem         powerUpSystem;

    // 4. Game loop (same structure as snake)
    while (true) {
        // input → game logic → physics → collision → particles → render → UI → present
    }
}
```
