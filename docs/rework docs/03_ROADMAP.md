# Rosario — Development Roadmap

> Phase-by-phase execution plan. Each phase has a clear goal, concrete tasks, a definition of done, and notes on what it unlocks.

---

## Phase 0 — Repository Restructure
**Goal:** Establish the engine/game directory split without changing any logic. The binary produced at the end of this phase is identical to the binary today.

**Estimated effort:** 1–2 focused sessions.

### Tasks

#### 0.1 — Create directory skeleton
```bash
mkdir -p engine/{ecs,systems,components,ui,postprocessing,animations,core}
mkdir -p games/snake/{components,systems,arena,collision,helpers}
mkdir -p games/pong
mkdir -p data/{snake,pong}
```

#### 0.2 — Move engine-level files

| Current location | New location |
|---|---|
| `srcs/ecs/Entity.hpp` | `engine/ecs/Entity.hpp` |
| `srcs/ecs/ComponentPool.hpp` | `engine/ecs/ComponentPool.hpp` |
| `srcs/ecs/Registry.hpp/.cpp` | `engine/ecs/Registry.hpp/.cpp` |
| `srcs/systems/InputSystem.*` | `engine/systems/InputSystem.*` |
| `srcs/systems/RenderSystem.*` | `engine/systems/RenderSystem.*` |
| `srcs/systems/PostProcessingSystem.*` | `engine/systems/PostProcessingSystem.*` |
| `srcs/systems/ParticleSystem.*` | `engine/systems/ParticleSystem.*` |
| `srcs/systems/AnimationSystem.*` | `engine/systems/AnimationSystem.*` |
| `srcs/systems/UIInteractionSystem.*` | `engine/systems/UIInteractionSystem.*` |
| `srcs/ui/` (entire subtree) | `engine/ui/` |
| `srcs/postprocessing/` | `engine/postprocessing/` |
| `srcs/animations/` | `engine/animations/` |
| `incs/FrameContext.hpp` | `engine/core/FrameContext.hpp` |
| `incs/DataStructs.hpp` | `engine/core/DataStructs.hpp` |

#### 0.3 — Move snake-specific files

| Current location | New location |
|---|---|
| `srcs/systems/MovementSystem.*` | `games/snake/systems/MovementSystem.*` |
| `srcs/systems/CollisionSystem.*` | `games/snake/systems/CollisionSystem.*` |
| `srcs/systems/AISystem.*` | `games/snake/systems/AISystem.*` |
| `srcs/AI/` (entire subtree) | `games/snake/AI/` |
| `srcs/arena/` | `games/snake/arena/` |
| `srcs/collision/` | `games/snake/collision/` |
| `srcs/helpers/` | `games/snake/helpers/` |
| `srcs/components/` | `games/snake/components/` |
| `srcs/main.cpp` | `games/snake/main.cpp` |

#### 0.4 — Move data files
```bash
mv data/CollisionRules.json      data/snake/
mv data/AIPresets.json           data/snake/
mv data/ArenaPresets.json        data/snake/
# Shared configs stay in data/ root or get duplicated per-game as needed:
mv data/ParticleConfig.json      data/snake/
mv data/PostProcessConfig.json   data/snake/
mv data/TunnelConfig.json        data/snake/
mv data/ButtonConfig.json        data/snake/
mv data/GlyphLibrary.json        data/snake/
mv data/GlyphPresets.json        data/snake/
```

#### 0.5 — Update all `#include` paths
Systematically update every `#include` in every moved file. The include search paths in the Makefile will need updating too (`-I engine -I games/snake` for the snake target).

Use a script to find broken includes:
```bash
grep -r '#include' engine/ games/ | grep -v '.d:' | sort
```

#### 0.6 — Update Makefile

Split `ALL_SRC` into `ENGINE_SRC`, `SNAKE_GAME_SRC`, `PONG_GAME_SRC`. Define `snake` and `pong` targets. `make all` builds both.

```makefile
SRCDIR := .   # files are now under engine/ and games/ directly

ENGINE_SRC := \
    engine/ecs/Registry.cpp \
    engine/systems/InputSystem.cpp \
    engine/systems/RenderSystem.cpp \
    engine/systems/PostProcessingSystem.cpp \
    engine/systems/ParticleSystem.cpp \
    engine/systems/AnimationSystem.cpp \
    engine/systems/UIInteractionSystem.cpp \
    engine/ui/TextSystem.cpp \
    engine/ui/MenuSystem.cpp \
    engine/ui/UISystem.cpp \
    engine/ui/ButtonConfigLoader.cpp \
    engine/ui/GlyphLibraryLoader.cpp \
    engine/ui/GlyphPresetLoader.cpp \
    engine/ui/PixelTextLayoutSystem.cpp \
    engine/ui/PixelTextRenderSystem.cpp \
    engine/ui/PixelTextHelper.cpp

SNAKE_GAME_SRC := \
    games/snake/systems/MovementSystem.cpp \
    games/snake/systems/CollisionSystem.cpp \
    games/snake/systems/AISystem.cpp \
    games/snake/AI/FloodFill.cpp \
    games/snake/AI/GridHelper.cpp \
    games/snake/AI/Pathfinder.cpp \
    games/snake/AI/AIPresetLoader.cpp \
    games/snake/arena/ArenaGrid.cpp \
    games/snake/arena/ArenaPresetLoader.cpp \
    games/snake/collision/CollisionRuleLoader.cpp \
    games/snake/collision/CollisionEffects.cpp \
    games/snake/collision/CollisionEffectDispatcher.cpp \
    games/snake/helpers/RaylibColors.cpp \
    games/snake/helpers/Factories.cpp \
    games/snake/helpers/GameManager.cpp \
    games/snake/helpers/StateTransitionHelper.cpp \
    games/snake/helpers/MenuLogoParticleHelper.cpp \
    games/snake/main.cpp

SNAKE_SRCS  := $(ENGINE_SRC) $(SNAKE_GAME_SRC)
SNAKE_OBJS  := $(addprefix $(OBJDIR)/snake/, $(SNAKE_SRCS:.cpp=.o))

snake: $(RAYLIB_SRC_DIR)/libraylib.a $(SNAKE_OBJS)
    $(CC) $(CFLAGS) $(SNAKE_OBJS) -o snake $(ALL_LIBS)
```

#### 0.7 — Verify compile and run
```bash
make snake
./snake   # must be identical to before
make test # all existing tests pass
```

### Definition of Done
- `make snake` compiles without errors or warnings
- `./snake` runs identically to before
- `make test` passes
- No source files remain in the old `srcs/` location (it can be removed)
- No game-specific code lives under `engine/`

---

## Phase 1 — Generalize FrameContext
**Goal:** Remove snake-specific assumptions from the engine's shared context type.

**Estimated effort:** Half a session (surgical, small change).

### Tasks

#### 1.1 — Create base FrameContext in `engine/core/`
Remove `gridWidth`, `gridHeight`, `menuLikeFrame`, and `ArenaGrid* arena` from the base struct. These move to `SnakeFrameContext`.

```cpp
// engine/core/FrameContext.hpp
struct FrameContext {
    GameState*  state       = nullptr;
    RenderMode* renderMode  = nullptr;
    ArenaBounds arenaBounds = {};
    float       gameAreaX   = 0.f;
    float       gameAreaY   = 0.f;
    int         cellSize    = 0;
    bool        playerDied  = false;
};
```

#### 1.2 — Create SnakeFrameContext in `games/snake/`
```cpp
// games/snake/SnakeFrameContext.hpp
struct SnakeFrameContext : public FrameContext {
    ArenaGrid*  arena         = nullptr;
    int         gridWidth     = 0;
    int         gridHeight    = 0;
    bool        menuLikeFrame = false;
};
```

#### 1.3 — Update all snake systems to use SnakeFrameContext
Replace `const FrameContext& ctx` with `const SnakeFrameContext& ctx` in:
- `CollisionSystem::update()`
- `AISystem::update()`
- `MovementSystem::update()` (if it uses ctx)
- `RenderSystem::render2D()`, `drawArena2D()`, etc.
- `snake/main.cpp`

Engine systems (`ParticleSystem`, `AnimationSystem`) keep `const FrameContext& ctx` — they only use the base fields.

#### 1.4 — Verify compile

### Definition of Done
- Engine headers have zero references to `ArenaGrid`, `gridWidth`, `gridHeight`, `menuLikeFrame`
- Snake compiles and runs identically
- Tests pass

---

## Phase 2 — PhysicsComponent + PhysicsSystem
**Goal:** Add continuous-space physics to the engine. No game uses it yet — build and test in isolation.

**Estimated effort:** 1–2 sessions.

### Tasks

#### 2.1 — PhysicsComponent
```cpp
// engine/components/PhysicsComponent.hpp
enum class PhysicsBehaviour { Bounce, Block, Trigger, None };

struct PhysicsComponent {
    float x, y;
    float width, height;
    float vx = 0.f, vy = 0.f;
    float restitution = 1.f;
    float mass = 1.f;
    float speed = 300.f;
    bool  isStatic = false;
    PhysicsBehaviour behaviour = PhysicsBehaviour::Bounce;
    bool  enabled = true;
};
```

#### 2.2 — TransformComponent
```cpp
// engine/components/TransformComponent.hpp
struct TransformComponent {
    float x = 0.f, y = 0.f;
    float rotation = 0.f;    // degrees
    float scaleX = 1.f, scaleY = 1.f;
};
```

#### 2.3 — PhysicsSystem skeleton
```cpp
// engine/systems/PhysicsSystem.hpp
class PhysicsSystem {
public:
    void update(Registry& registry, float dt, const FrameContext& ctx);
private:
    void integrate(Registry& registry, float dt);
    void resolveAABB(PhysicsComponent& a, PhysicsComponent& b);
};
```

#### 2.4 — Implement AABB collision detection
Basic separating axis test:
```cpp
bool AABBOverlap(const PhysicsComponent& a, const PhysicsComponent& b) {
    return a.x < b.x + b.width  &&
           a.x + a.width  > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}
```

#### 2.5 — Implement bounce response
For ball (dynamic) vs. wall (static):
- Determine collision axis (which axis has less overlap)
- Reflect velocity on that axis
- Apply restitution

#### 2.6 — Write unit tests for PhysicsSystem
```cpp
// tests/unit/PhysicsSystemTest.cpp
TEST(PhysicsSystem, BallBouncesOffTopWall) { ... }
TEST(PhysicsSystem, BallBouncesOffPaddle) { ... }
TEST(PhysicsSystem, StaticBodyDoesNotMove) { ... }
TEST(PhysicsSystem, RestitutionReducesSpeed) { ... }
```

### Definition of Done
- `PhysicsSystem` unit tests pass
- No integration into any game yet (that's Phase 4)
- PhysicsComponent and TransformComponent headers are in `engine/components/`

---

## Phase 3 — Pong Arena + Preset System
**Goal:** Build the `PongArena` class and its JSON preset loader. Get the `AnimationSystem` feeding off Pong arena outlines.

**Estimated effort:** 1–2 sessions.

### Tasks

#### 3.1 — PongArena class
```cpp
// games/pong/arena/PongArena.hpp
struct WallRect { float x, y, width, height; };

class PongArena {
    std::vector<WallRect>             _walls;
    std::vector<std::vector<Vector2>> _outlines;
    std::string                       _name;
public:
    void loadPreset(const PongArenaPreset& preset);
    const std::vector<WallRect>& getWalls() const;
    std::vector<std::vector<Vector2>> getAllOutlines() const;
    bool isBallOutOfBounds(float x, float y, float w, float h) const;
    int  getScoringEdge(float x, float y) const; // -1 = left out, 1 = right out, 0 = not out
};
```

#### 3.2 — PongArenaPresetLoader
JSON format:
```json
{
  "presets": [{
    "name": "Classic",
    "walls": [
      { "x": 0, "y": 0, "width": 1920, "height": 20 },
      { "x": 0, "y": 1060, "width": 1920, "height": 20 }
    ],
    "outline": [[0,0],[1920,0],[1920,1080],[0,1080]]
  }]
}
```

#### 3.3 — Create data/pong/ArenaPresets.json
Start with 2 presets: Classic and The Fangs. Add others later.

#### 3.4 — Verify AnimationSystem works with Pong outlines
Write a minimal test that creates a `PongArena`, calls `getAllOutlines()`, feeds it into `AnimationSystem::notifyShapeOverride()`, and checks that lines render. This proves the engine system is truly game-agnostic.

#### 3.5 — PongFrameContext
```cpp
// games/pong/PongFrameContext.hpp
struct PongFrameContext : public FrameContext {
    PongArena* arena     = nullptr;
    float      arenaW    = 1920.f;
    float      arenaH    = 1080.f;
};
```

### Definition of Done
- `PongArena` loads JSON presets correctly
- `AnimationSystem` renders the tunnel effect using Pong outlines
- `PongFrameContext` compiles

---

## Phase 4 — Pong Core: Ball, Paddles, Scoring
**Goal:** A working Pong game. No power-ups, no arena shapes (Classic only), no particles. Just the core game loop.

**Estimated effort:** 2–3 sessions.

### Tasks

#### 4.1 — Pong-specific components
Create all components listed in the Pong Design Document:
- `BallComponent`
- `PaddleComponent`

#### 4.2 — Paddle input and movement
`PaddleSystem` reads held keys (W/S for left, Up/Down for right). Applies velocity to `PhysicsComponent`. Clamps to arena bounds.

**Note on InputSystem:** The current `InputSystem` uses buffered `IsKeyPressed()`. Paddles need `IsKeyDown()`. Options:
- Add a `continuous` flag to `InputComponent` and handle in `InputSystem`
- `PaddleSystem` bypasses `InputSystem` and calls Raylib directly

The second option is simpler and honest — `PaddleSystem` owns paddle input entirely.

#### 4.3 — Ball launch
On game start / round reset: spawn ball at center, launch at random angle ±45° from horizontal, alternate serve direction each round.

#### 4.4 — PongCollisionSystem
AABB checks: ball vs. top/bottom walls (bounce), ball vs. paddles (deflect with angle), ball vs. left/right edges (score event).

Use same `CollisionEffectDispatcher` pattern:
```cpp
dispatcher.register("ballPaddleDeflect", effectBallPaddleDeflect);
dispatcher.register("ballWallBounce",    effectBallWallBounce);
dispatcher.register("ballScore",         effectBallScore);
```

#### 4.5 — BallSystem (basic)
Speed ramp on paddle hit. Round reset on score. Score tracking in a simple game state struct (or in the UI entity as TS did it).

#### 4.6 — Pong main.cpp
Full game loop following the pattern from snake's main. Menu → Playing → GameOver state machine. Menu uses existing `UIInteractionSystem` + `ButtonConfigLoader` with a new `data/pong/ButtonConfig.json`.

#### 4.7 — Score UI
Use `PixelTextComponent` + `PixelTextRenderSystem` for score display. This is the engine's UI system used identically to snake.

#### 4.8 — Integration test
```bash
make pong
./pong   # ball bounces, paddles move, score increments, game over screen appears
```

### Definition of Done
- Ball launches, bounces off walls and paddles with angle variation
- Both paddles respond to input
- Score increments on out-of-bounds ball
- Menu → Playing → GameOver flow works
- CRT post-processing renders on Pong (same as snake — just needs `ppSystem.setConfig(ppPresets.at("crt_bloom"))`)
- `make pong` produces a clean binary

---

## Phase 5 — Pong Arena Shapes
**Goal:** All named arena presets from the original, with animated transitions.

**Estimated effort:** 1–2 sessions.

### Tasks

#### 5.1 — Implement remaining presets in JSON
Complete `data/pong/ArenaPresets.json` with:
- Classic
- The Steps
- The Bowtie
- The Fangs
(Add more as desired — each is just a list of wall rects + outline polygon)

#### 5.2 — Arena cycling in gameplay
Add TAB key (or options menu) to cycle arena presets. Call `animationSystem.notifyArenaSpawning()` and `arena.loadPreset()` — identical to snake's arena cycling.

#### 5.3 — Wall collision with arena shapes
Ensure `PongCollisionSystem` tests ball against all `WallRect` entries from `PongArena::getWalls()`, not just hardcoded top/bottom.

#### 5.4 — Verify AnimationSystem transitions
Shape transitions should look identical to snake's arena transitions. The engine system handles this already — just verify Pong outlines produce the expected visual.

### Definition of Done
- All named arenas load and display correctly
- Arena shape transitions animate smoothly
- Ball collides correctly with non-Classic wall shapes
- TAB cycles between arenas in-game

---

## Phase 6 — Particles + Visual Polish
**Goal:** Ball trail, score explosion, menu particle effects. Pong should look like it belongs to the same visual universe as snake.

**Estimated effort:** 1 session.

### Tasks

#### 6.1 — Ball trail
`PongRenderSystem` (or `BallSystem`) emits `ParticleSpawnRequest` with type `Trail` at ball position each frame. Color matches ball type:
- Normal ball → yellow (same as food color in snake)
- Arrow ball → red/orange burst
- Egg ball → soft white trail

#### 6.2 — Score event explosion
When a point is scored, emit `ParticleSpawnRequest` with type `Explosion` at the scoring edge.

#### 6.3 — Menu particle dust
Same ambient dust as snake menu — `ParticleSystem` handles this via `spawnDust()`, already called automatically in `update()`.

#### 6.4 — Power-up collect burst
On `Ball × PowerUp` collision, emit explosion particles in the power-up's color.

#### 6.5 — Paddle hit flash
Brief visual feedback on paddle contact — a short particle burst or color flash on the paddle entity.

### Definition of Done
- Ball has a visible trail during play
- Score events produce a particle burst
- Menu has ambient dust
- Game looks visually consistent with snake

---

## Phase 7 — Power-Ups, Power-Downs, Ball Changes
**Goal:** The full power-up ecosystem from the original Pong.

**Estimated effort:** 3–4 sessions (largest single phase).

### Tasks

#### 7.1 — PowerUpComponent and PowerUpSystem skeleton
Spawning timer, random position, entity creation with a visual icon.

#### 7.2 — Implement power-ups (positive)
In order of implementation complexity:

1. **Double Paddle Size** — modify `PaddleComponent::sizeMultiplier`, update `PhysicsComponent::height`
2. **Shield** — spawn a `ShieldComponent` entity behind the paddle
3. **Attract Balls** — in `PaddleSystem`, if `attractsBalls` flag, apply force toward paddle each frame
4. **Shooter** — in `PaddleSystem`, on fire key, spawn 3 `ProjectileComponent` entities

#### 7.3 — Implement power-downs (negative)
1. **Half Paddle Size** — same as DoublePaddleSize but opposite multiplier
2. **Slow Paddle** — speed modifier in `PaddleSystem`
3. **Invert Controls** — flip velocity direction in `PaddleSystem`
4. **Flatten Returns** — in `PongCollisionSystem` deflection, reduce max angle when flag set

#### 7.4 — Implement ball changes
1. **Arrow Ball** — set `BallComponent::type = Arrow`, increase speed, update render
2. **Multi Ball** — spawn N additional ball entities (some `isDecoy = true`)
3. **Square Ball** — set type, enable `TransformComponent::rotation` increment in `BallSystem`
4. **Egg Ball** — set type, enable `curvature` application in `PhysicsSystem`

#### 7.5 — Affectation timer + visual bar
`PaddleSystem` decrements `PaddleComponent::affectationTimer` each frame. When it reaches 0, revert the effect. Render a small bar on the paddle's side of the screen (like TS original) using `DrawRectangle`.

#### 7.6 — PowerUpSystem collision registration
Register all effects in the `CollisionEffectDispatcher`:
```cpp
dispatcher.register("collectPowerUp",  effectCollectPowerUp);
dispatcher.register("collectPowerDown", effectCollectPowerDown);
dispatcher.register("collectBallChange", effectCollectBallChange);
```

### Definition of Done
- All 4 power-ups implemented and revert correctly
- All 4 power-downs implemented and revert correctly
- All 4 ball changes implemented
- Affectation timer bar renders on screen
- No power-up effect persists past its timer

---

## Phase 8 — Spawning Mid-Arena Obstacles
**Goal:** Dynamic obstacles that spawn in the arena center during play, as per the original.

**Estimated effort:** 1–2 sessions.

### Tasks

#### 8.1 — ObstacleComponent
```cpp
struct ObstacleComponent {
    float spawnTimer   = 0.f;
    float lifetime     = 0.f;
    bool  isDespawning = false;
};
```

#### 8.2 — ObstacleSystem
Spawns obstacles at configured intervals. Each obstacle is a `PhysicsComponent(isStatic=true)` + `ObstacleComponent` + `RenderComponent`. Ball collision handled by existing `PongCollisionSystem` (ball vs. any static physics entity → bounce).

#### 8.3 — Obstacle shapes
Start with rectangular obstacles (simple AABB). Named shapes ("The Cross", "The Grid", etc.) defined in a JSON preset or as hardcoded patterns per arena.

#### 8.4 — Spawn/despawn animation
Reuse the `ArenaGrid` spawn/despawn alpha fade pattern: obstacle fades in over N frames, then fades out before removal. This gives visual coherence with snake's wall animations.

### Definition of Done
- Obstacles spawn periodically during gameplay
- Ball bounces off obstacles correctly
- Obstacles fade in/out visually
- Obstacles are configurable per-arena preset

---

## Phase 9 — Snake Polish Pass
**Goal:** With the engine now fully separated, review snake for any remaining rough edges and add anything that was deferred.

**Estimated effort:** 1–2 sessions.

### Tasks

#### 9.1 — Audit snake for any engine code it's carrying
After the Phase 0 restructure, check that no snake system is doing work that should be in the engine.

#### 9.2 — Review remaining debug output
The `CollisionSystem` has `std::cout` debug prints. Remove or gate behind a debug flag.

#### 9.3 — Production build flag
The Makefile has production flags commented out. Enable them for a release build target:
```makefile
release_snake: CFLAGS := -std=c++20 -O2 -Wall -Wextra -Werror -Wno-unused-parameter $(INCLUDES)
release_snake: snake
```

#### 9.4 — Options menu
Both snake and pong have an OPTIONS button in the menu. Implement the options screen (currently likely a stub): volume, render mode toggle (2D/3D for snake), post-processing toggle.

#### 9.5 — Test coverage expansion
Add integration tests for the full snake game loop: entity spawning, movement ticks, collision detection, score tracking.

---

## Phase 10 — Documentation + Portfolio Presentation
**Goal:** Make the repository legible to someone who has never seen it before and is evaluating it in 10 minutes.

**Estimated effort:** 1 session.

### Tasks

#### 10.1 — Root README.md
```markdown
# Rosario Engine
Screenshot of snake. Screenshot of pong.
One paragraph on what it is.
Build instructions (make snake, make pong).
Engine feature list.
Architecture overview diagram (text-based is fine).
```

#### 10.2 — engine/README.md
Brief description of each system, with links to the architecture reference.

#### 10.3 — Code comments audit
Ensure every system class has a header comment explaining its responsibility and usage contract.

#### 10.4 — Demo recording
Record a short screen capture of both games running. Useful for GitHub README and for sharing directly.

---

## Milestone Summary

| Milestone | Phase(s) | What exists after |
|---|---|---|
| **Restructured repo** | 0–1 | Snake works, split architecture, clean Makefile |
| **Engine extended** | 2 | PhysicsSystem + new components, tested in isolation |
| **Pong arena** | 3 | Tunnel animation works with Pong shapes |
| **Pong playable** | 4 | Core Pong game loop, two players, scoring, menus |
| **Pong complete arenas** | 5 | All named arena shapes with transitions |
| **Pong visual polish** | 6 | Particles, trails, effects — same visual identity as snake |
| **Pong full features** | 7–8 | All power-ups, ball types, obstacles |
| **Both games production** | 9–10 | Polish, docs, portfolio-ready |

---

## Risk Register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Include path hell after restructure | High | Medium | Fix methodically file by file; use the grep script |
| PhysicsSystem feel is wrong (Pong not fun) | Medium | High | Implement ball-paddle angle deflection correctly from day 1 — this is the make-or-break mechanic |
| Arena wall shapes in continuous space are imprecise | Medium | Low | AABB approximations are fine for most shapes; diagonal segment collision can be added later |
| Power-up interactions create unexpected bugs | Medium | Medium | Test each power-up independently before combining |
| Makefile complexity gets unwieldy | Low | Low | Add a target at a time; keep ENGINE_SRC clearly separated |
| FrameContext inheritance causes slicing bugs | Low | High | Always pass game context by reference, never by value |

---

## Development Principles

**One thing at a time.** Each phase ends with a compiling, running binary. Never leave the project in a broken state overnight.

**Engine code is sacred.** If you find yourself adding game logic to an engine file, stop and create a game-level system instead.

**The visual identity is the constraint.** Both games must run through the same post-processing pipeline. Don't prototype Pong without CRT — it will look wrong and feel disconnected.

**Tests for physics, not for rendering.** Write unit tests for `PhysicsSystem` math (bounce angles, AABB detection, restitution). Don't try to test rendering.

**JSON first.** Before hardcoding any value (arena layout, particle config, power-up timing), ask whether it should be a JSON config. If you'd ever want to tune it, it should be data.
