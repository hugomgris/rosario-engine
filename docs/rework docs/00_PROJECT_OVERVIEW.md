# Rosario Engine — Project Overview

> A C++ game engine built on Raylib, developed in parallel with two arcade games that showcase its capabilities.

---

## Vision

Rosario is not generic middleware. It is an **opinionated creative toolkit** with a distinct visual identity: retro-aesthetic, heavily animated, particle-rich arcade experiences running through a CRT post-processing pipeline. The engine's design philosophy is visible in every game built on it.

The portfolio statement is specific: two games of different genres (snake, pong), both expressing the same visual language, powered by one shared technical foundation. That coherence across genres is what makes this interesting to a game dev hiring team.

---

## The Three Projects

### 1. Rosario Engine (`engine/`)
The shared foundation. ECS core, rendering pipeline, particle system, animation system, post-processing, UI infrastructure, physics. No game logic. No snake. No pong.

**Target audience for portfolio:** Engine programmer, gameplay programmer, and generalist game dev roles.

**What it demonstrates:**
- Typed component pool ECS with variadic `view<>()` queries
- Data-driven configuration (JSON loaders for particles, post-process, arena presets, glyphs)
- Multi-pass post-processing pipeline (bloom extract → blur → CRT composite)
- Particle system with typed emitters and per-frame request queue
- Outline-driven animation system working with arbitrary polygon shapes
- Pixel-font glyph rendering system with hot-reload
- Generic input system supporting multiple player slots
- Continuous-space physics system (AABB, bounce, restitution)

### 2. Snake (`games/snake/`)
The original test subject. Grid-based movement, AI pathfinding (A* + flood fill), data-driven collision rules, arena presets with animated transitions, multi-player support.

**What it demonstrates on top of the engine:**
- Grid-based ECS architecture
- AI system (BFS pathfinder + flood fill safety check + survival mode)
- Data-driven collision effect dispatch
- Arena grid with spawn/despawn animation pipeline

### 3. Pong (`games/pong/`)
The proof of engine reusability. A full arcade Pong port from a TypeScript/Pixi.js original, rebuilt natively using the Rosario engine. Continuous-space physics, power-up/power-down system, multiple ball types, morphing arena walls, spawning obstacles.

**What it demonstrates on top of the engine:**
- Continuous-space physics on the same ECS that runs a grid game
- Complex game state driven by component composition (power-up effects via component add/remove)
- Same visual pipeline (particles, CRT, animation) applied to a completely different game genre
- Data-driven arena shape presets in continuous space

---

## Shared Visual Identity

Both games share:

| Feature | Snake | Pong |
|---|---|---|
| CRT + Bloom post-processing | ✓ | ✓ |
| Particle trail system | ✓ | ✓ |
| Outline tunnel animation | ✓ | ✓ |
| Pixel-font glyph UI | ✓ | ✓ |
| JSON-driven configuration | ✓ | ✓ |
| Named arena presets | ✓ | ✓ |
| Animated arena transitions | ✓ | ✓ |
| Two-player local support | ✓ | ✓ |
| Data-driven collision effects | ✓ | ✓ |

---

## Repository Structure (Target)

```
rosario/
├── engine/               # Shared engine — no game logic
│   ├── ecs/
│   ├── systems/
│   ├── components/
│   ├── ui/
│   ├── postprocessing/
│   ├── animations/
│   └── core/
├── games/
│   ├── snake/            # Snake game — engine consumer
│   └── pong/             # Pong game — engine consumer
├── incs/                 # Shared low-level types (Vec2, Colors, etc.)
├── libs/                 # Raylib, GoogleTest (gitignored built artifacts)
├── shaders/              # GLSL shaders shared by all games
├── data/
│   ├── snake/            # Snake-specific JSON configs
│   └── pong/             # Pong-specific JSON configs
├── tests/                # Unit + integration tests
└── Makefile              # Multi-target: make snake, make pong, make all
```

---

## Build Targets

```bash
make snake      # Build snake game binary
make pong       # Build pong game binary
make all        # Build both
make test       # Run GoogleTest suite
make info       # Print build configuration
```

---

## Key Design Rules

1. **Engine code never includes game code.** The dependency arrow points one way: games → engine.
2. **All configuration is JSON.** Collision rules, particle configs, arena presets, glyph libraries, post-process profiles — none of this is hardcoded.
3. **Systems are stateless where possible.** State lives in components, not in system member variables.
4. **FrameContext is the system bus.** Read-only world state passed into every system update. Game-specific context extends the base struct via inheritance.
5. **The visual identity is non-negotiable.** Every game built on Rosario runs through the same post-processing pipeline.
