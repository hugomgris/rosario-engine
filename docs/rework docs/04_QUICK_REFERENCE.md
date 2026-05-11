# Rosario — Quick Reference

A fast-access cheat sheet for daily development. The full documents are in `00_PROJECT_OVERVIEW.md`, `01_ENGINE_ARCHITECTURE.md`, `02_PONG_DESIGN.md`, and `03_ROADMAP.md`.

---

## Build Commands

```bash
make snake          # Build snake
make pong           # Build pong
make all            # Build both
make test           # Run GoogleTest suite
make re             # Clean + build all
make info           # Print build config

./snake             # Run snake
./pong              # Run pong

make gamecheck      # Valgrind snake
```

---

## In-Game Debug Keys

| Key | Effect |
|---|---|
| `F` | Toggle fullscreen |
| `1` | 2D render mode |
| `2` | 3D render mode |
| `P` | Toggle post-processing |
| `F8` | Toggle particle debug overlay |
| `F9` | Hot-reload glyph library |
| `F10` | Toggle layout debug overlay |
| `TAB` | Cycle arena presets |

---

## The Dependency Rule

```
games/snake/  ──┐
                ├──▶  engine/  ──▶  libs/ (raylib)
games/pong/   ──┘
```

- Game code imports from `engine/` — always fine
- Engine code NEVER imports from `games/` — forbidden
- If you're tempted to add game logic to an engine file, make a new game-level system

---

## Adding a New Component

1. Create header in `engine/components/` (engine-level) or `games/<game>/components/` (game-level)
2. No registration needed — `ComponentPool<T>` is created lazily on first `addComponent<T>()`
3. Add to `view<>()` calls in systems that need it

```cpp
// Header
struct MyComponent {
    float value = 0.f;
    bool  active = true;
};

// Usage
registry.addComponent<MyComponent>(entity, { .value = 42.f });
auto& c = registry.getComponent<MyComponent>(entity);
```

---

## Adding a New System

1. Create `.hpp` / `.cpp` in `engine/systems/` or `games/<game>/systems/`
2. Add `.cpp` to `ENGINE_SRC` or `<GAME>_GAME_SRC` in Makefile
3. Instantiate in the game's `main.cpp`
4. Call `update()` in the correct position in the game loop

Systems take `Registry&`, `float dt`, and `const FrameContext&` (or game-specific context).

---

## Adding a New Collision Effect (Snake)

1. Implement the function in `games/snake/collision/CollisionEffects.cpp`:
```cpp
void effectMyThing(Registry& reg, Entity subject, Entity object, FrameContext& ctx) {
    // ...
}
```
2. Register in `CollisionEffectDispatcher::registerDefaults()`:
```cpp
register("myThing", effectMyThing);
```
3. Add the effect name to the relevant rule in `data/snake/CollisionRules.json`:
```json
{ "subject": "Snake", "object": "Food", "effects": ["grow", "myThing", "spawnFood"] }
```

---

## Adding a New Collision Effect (Pong)

Same pattern — different files:
1. `games/pong/collision/PongCollisionEffects.cpp`
2. Register in Pong's dispatcher setup in `main.cpp`
3. Add to `data/pong/PongCollisionRules.json`

---

## FrameContext Fields

### Base (all games)
| Field | Type | Description |
|---|---|---|
| `state` | `GameState*` | Current game state |
| `renderMode` | `RenderMode*` | 2D or 3D |
| `arenaBounds` | `ArenaBounds` | Screen-space rect of full arena |
| `gameAreaX/Y` | `float` | Inner play area origin |
| `cellSize` | `int` | Pixels per grid cell (0 if continuous) |
| `playerDied` | `bool` | Set by collision system, checked after update |

### Snake extension (SnakeFrameContext)
| Field | Type | Description |
|---|---|---|
| `arena` | `ArenaGrid*` | Current arena grid |
| `gridWidth/Height` | `int` | Grid dimensions |
| `menuLikeFrame` | `bool` | True in Menu and GameOver states |

### Pong extension (PongFrameContext)
| Field | Type | Description |
|---|---|---|
| `arena` | `PongArena*` | Current pong arena |
| `arenaW/H` | `float` | Arena dimensions in pixels |

---

## ECS Patterns

```cpp
// Iterate entities with multiple components
for (auto entity : registry.view<ComponentA, ComponentB>()) {
    auto& a = registry.getComponent<ComponentA>(entity);
    auto& b = registry.getComponent<ComponentB>(entity);
}

// Safe iteration with mid-loop removal
registry.forEach<MyComponent>([&](Entity e, MyComponent& c) {
    if (c.expired)
        registry.removeComponent<MyComponent>(e);  // safe — forEach snapshots first
});

// Check component existence
if (registry.hasComponent<InputComponent>(entity)) { ... }

// Destroy entity (removes from all pools)
registry.destroyEntity(entity);
```

---

## Particle Spawning

Systems never call `ParticleSystem` directly — they add a request component:

```cpp
// In any system or render function:
ParticleSpawnRequest req;
req.type       = pType::Explosion;
req.x          = screenX;
req.y          = screenY;
req.color      = someColor;
// req.gridCoords = false (default — x/y already in screen space)
registry.addComponent<ParticleSpawnRequest>(entity, req);

// ParticleSystem::update() consumes all requests each frame
```

For interval-based emitters (menu trails):
```cpp
req.type          = pType::MenuTrail;
req.spawnInterval = 0.05f;       // emit every 50ms
req.emitterKey    = someUniqueId; // per-emitter timer key
req.direction     = Direction::RIGHT;
```

---

## Arena Preset Workflow

### Snake
1. Add entry to `data/snake/ArenaPresets.json`
2. `ArenaPresetLoader::load()` picks it up automatically
3. TAB key cycles to it in-game

### Pong
1. Add entry to `data/pong/ArenaPresets.json` (wall rects + outline polygon)
2. `PongArenaPresetLoader::load()` picks it up
3. `PongArena::loadPreset()` updates wall data + outlines
4. `animationSystem.notifyArenaSpawning()` triggers transition

---

## Post-Processing Profiles

Defined in `data/<game>/PostProcessConfig.json`. Key fields:
```json
{
  "name": "crt_bloom",
  "effects": ["CRT", "Bloom"],
  "scanlineIntensity": 0.15,
  "curvatureAmount": 0.03,
  "vignetteStrength": 0.4,
  "chromaticAberration": 0.002,
  "grainAmount": 0.03,
  "bloomIntensity": 1.2
}
```

Load and apply:
```cpp
auto ppPresets = PostProcessConfigLoader::load("data/snake/PostProcessConfig.json");
ppSystem.setConfig(ppPresets.at("crt_bloom"));
```

---

## Phase Status Tracker

| Phase | Description | Status |
|---|---|---|
| 0 | Repository restructure | ⬜ Not started |
| 1 | Generalize FrameContext | ⬜ Not started |
| 2 | PhysicsComponent + PhysicsSystem | ⬜ Not started |
| 3 | PongArena + preset system | ⬜ Not started |
| 4 | Pong core (ball, paddles, scoring) | ⬜ Not started |
| 5 | Pong arena shapes | ⬜ Not started |
| 6 | Pong particle polish | ⬜ Not started |
| 7 | Power-ups, power-downs, ball changes | ⬜ Not started |
| 8 | Spawning obstacles | ⬜ Not started |
| 9 | Snake polish pass | ⬜ Not started |
| 10 | Docs + portfolio presentation | ⬜ Not started |

Update status as you go: ⬜ Not started → 🔄 In progress → ✅ Done
