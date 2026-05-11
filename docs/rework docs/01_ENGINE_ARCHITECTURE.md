# Rosario Engine — Architecture Reference

---

## ECS Core

### Entity (`engine/ecs/Entity.hpp`)
A dumb ID wrapper. Entities have no behavior, no component storage, no knowledge of what components they carry. Just a `uint32_t` with equality operators.

```cpp
class Entity {
    using ID = std::uint32_t;
    ID _id;
public:
    explicit Entity(ID id);
    ID getID() const;
    bool operator==(const Entity&) const;
    bool operator!=(const Entity&) const;
};
```

### ComponentPool (`engine/ecs/ComponentPool.hpp`)
Typed, packed-array storage for one component type. Uses a swap-with-last removal strategy to keep the array contiguous. Provides O(1) add, O(1) get, O(1) remove (amortized).

```
_components[]     — packed array of T
_entityList[]     — parallel array of owning entities
_entityToIndex{}  — entity ID → index lookup
```

The `IComponentPool` base class allows the Registry to hold pools of any type via `shared_ptr<IComponentPool>`.

### Registry (`engine/ecs/Registry.hpp / .cpp`)
The central entity/component store. Key design points:

- **Lazy pool creation** — pools are created on first `addComponent<T>()` call, never upfront
- **Type-indexed storage** — `std::unordered_map<std::type_index, shared_ptr<IComponentPool>>`
- **`view<T...>()`** — variadic template query; iterates the smallest pool first, filters by remaining types. Returns `vector<Entity>` snapshot.
- **`forEach<T>(fn)`** — takes a snapshot of the pool's entity list before iteration to handle mid-loop removal safely
- **`destroyEntity()`** — removes from all pools via the `IComponentPool` virtual interface

```cpp
// Usage examples
Entity e = registry.createEntity();
registry.addComponent<PositionComponent>(e, {0, 0});

for (auto entity : registry.view<SnakeComponent, PositionComponent>()) {
    auto& pos = registry.getComponent<PositionComponent>(entity);
}

registry.forEach<ParticleSpawnRequest>([&](Entity e, ParticleSpawnRequest& req) {
    // safe even if req is removed inside
    registry.removeComponent<ParticleSpawnRequest>(e);
});
```

---

## FrameContext

The system bus. A lightweight struct populated once per frame by `RenderSystem::fillContext()`, then passed read-only into every system's `update()`.

### Base context (`engine/core/FrameContext.hpp`)
```cpp
struct FrameContext {
    GameState*  state       = nullptr;
    RenderMode* renderMode  = nullptr;
    ArenaBounds arenaBounds = {};   // screen-space arena rect
    float       gameAreaX   = 0.f; // inner play area origin
    float       gameAreaY   = 0.f;
    int         cellSize    = 0;   // pixels per grid cell (0 for continuous)
    bool        playerDied  = false;
};
```

### Game-specific extensions
Games extend the base struct. Engine systems only see `FrameContext`. Game systems use the derived type directly — no casting needed.

```cpp
// games/snake/SnakeFrameContext.hpp
struct SnakeFrameContext : public FrameContext {
    ArenaGrid*  arena         = nullptr;
    int         gridWidth     = 0;
    int         gridHeight    = 0;
    bool        menuLikeFrame = false;
};

// games/pong/PongFrameContext.hpp
struct PongFrameContext : public FrameContext {
    PongArena*  arena       = nullptr;
    float       arenaWidth  = 0.f;
    float       arenaHeight = 0.f;
};
```

---

## Engine Systems

### InputSystem
Polls Raylib for key events. Maps entities to `PlayerSlot` (A or B). Fills the entity's `InputComponent::inputBuffer` queue. Slot assignment happens at game startup; the system itself is slot-agnostic.

**Generic — both games use this.**

### RenderSystem
Handles window creation, 2D/3D camera management, layout calculation (arena bounds, cell size, game area offset), and `fillContext()`. Dispatches to `render2D()` or `render3D()` based on `RenderMode`.

Snake-specific draw calls (`drawSnakes2D`, `drawFood2D`, `drawArena2D`) live in the snake game directory, not in the engine's RenderSystem. The engine's RenderSystem provides the camera, layout math, and draw primitive helpers.

**Partially generic — layout and camera logic belongs to engine; draw calls belong to each game.**

### PostProcessingSystem
Multi-pass GPU post-processing pipeline:
1. Scene renders into `_renderTarget` (offscreen)
2. `bloom_extract.fs` downsamples to half-res, extracts bright pixels
3. `blur.fs` applied horizontally → `_pingPongBuffers[0]`
4. `blur.fs` applied vertically → `_pingPongBuffers[1]`
5. `crt_bloom.fs` composites scene + bloom buffer with CRT effects

Config-driven via `PostProcessConfig` (scanline intensity, curvature, vignette, chromatic aberration, grain, bloom intensity). Presets loaded from JSON.

**Fully generic — used identically by both games.**

### ParticleSystem
Typed particle emitter system. Particle types: `Dust`, `Explosion`, `Trail`, `MenuTrail`. Particles live in a flat `vector<Particle>` and are simulated each frame (position integration, drag, size fade, age-based death).

Game systems don't call spawn functions directly — they add a `ParticleSpawnRequest` component to an entity. `ParticleSystem::update()` consumes all pending requests each frame, then removes the component. This decouples the emitter from the physics/game logic.

Per-emitter timers stored in `_menuTrailEmitterTimers` map (keyed by `emitterKey` in the request) allow interval-based emission without coupling to the game loop.

**Fully generic — both games use this identically.**

### AnimationSystem
Renders the "tunnel" effect: concentric outline copies of the arena shape that appear to rush toward the viewer. Each `TunnelLine` has an `age`, `progress` (eased), and an `epoch` (used to transition between old and new shapes).

Shape input is a `vector<vector<Vector2>>` — arbitrary polygons in screen space. `ArenaGrid::getAllOutlines()` produces these for snake. Pong's `PongArena` will produce equivalent polygon outlines.

Key operations:
- `notifyArenaSpawning(arena)` — bumps epoch, old lines keep old shapes
- `notifyArenaDespawning(arena)` — triggers `despawnPending` callback
- `notifyShapeOverride(shapes)` — swap shapes without arena context
- `instantShapeChange(shapes)` — immediate swap, clears all lines

**Fully generic — the system doesn't know what game it's in.**

### UIInteractionSystem
Keyboard and mouse navigation for button menus. Maintains hover state, handles directional navigation (arrow keys / WASD), processes click and confirm events. Events are written to an `EventQueue` consumed by the main loop.

Buttons are filtered by `ButtonMenu` enum value, ordered by `ButtonConfig::index`.

**Fully generic — Pong menus use this identically.**

### PhysicsSystem *(to be built)*
Continuous-space AABB physics. Integrates velocity, resolves AABB vs. AABB collisions, applies restitution. Ball-paddle collision computes deflection angle based on contact point relative to paddle center (the core Pong feel mechanic).

---

## Engine Components

### Existing (to be moved to `engine/components/`)
| Component | Description |
|---|---|
| `InputComponent` | Key event buffer queue |
| `RenderComponent` | Color, visibility flags |
| `ButtonComponent` | Bounds, hover state, menu membership |
| `ButtonActionComponent` | Action type enum |
| `ParticleSpawnRequest` | One-frame emitter request, consumed by ParticleSystem |
| `PixelTextComponent` | Glyph-based text, state visibility flags |
| `PixelTextLayoutComponent` | Layout dirty flag, computed positions |

### New (engine-level additions for Pong)
| Component | Description |
|---|---|
| `PhysicsComponent` | Position (float), velocity, AABB bounds, restitution, mass, `isStatic` |
| `TransformComponent` | Continuous x/y position + rotation angle (for spinning ball rendering) |

---

## UI Pipeline

The pixel-font UI pipeline is one of the engine's more distinctive features:

```
GlyphLibrary (JSON)          — bitmap pixel patterns for each character
    ↓
GlyphPresets (JSON)          — named text configs (font size, color, spacing)
    ↓
PixelTextComponent           — text content + visibility state config
    ↓
PixelTextLayoutSystem        — computes glyph screen positions, sets dirty flag
    ↓
PixelTextRenderSystem        — draws computed glyphs to screen
```

Hot-reload supported: `F9` key reloads both JSON files and re-applies to all active `PixelTextComponent` entities.

---

## Collision Architecture (Snake)

The snake collision system is data-driven:

```
CollisionRules.json
    ↓
CollisionRuleLoader          — builds CollisionRuleTable (subject × object → effects[])
    ↓
CollisionSystem              — detects collisions, calls dispatcher
    ↓
CollisionEffectDispatcher    — maps effect name string → registered function
    ↓
CollisionEffects.cpp         — concrete implementations (kill, grow, spawn food, etc.)
```

This pattern ports directly to Pong: `PongCollisionRules.json` maps `Ball × Paddle`, `Ball × Wall`, `Ball × PowerUp`, etc. to named effects registered in `PongCollisionEffects.cpp`.

---

## Data Flow Per Frame

```
1. Input polling          InputSystem::update()
                          → fills InputComponent buffers

2. AI (snake only)        AISystem::update()
                          → writes to MovementComponent::direction

3. Game logic update      MovementSystem / BallSystem / PaddleSystem
                          → mutates positions, velocities

4. Collision              CollisionSystem / PongCollisionSystem
                          → dispatches effects, may set playerDied, spawn particles

5. Particle update        ParticleSystem::update()
                          → consumes ParticleSpawnRequests, simulates particles

6. Animation update       AnimationSystem::update()
                          → advances tunnel line ages

7. Render (3D/2D)         RenderSystem
                          → draws arena, entities (inside PostProcessing capture)

8. UI render              MenuSystem / PixelTextRenderSystem
                          → draws UI on top (inside PostProcessing capture)

9. Post-process present   PostProcessingSystem::applyAndPresent()
                          → bloom → CRT → output to screen

10. HUD                   FPS counter, debug overlays (outside PostProcessing)
```
