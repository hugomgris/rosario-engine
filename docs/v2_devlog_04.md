# Rosario - Devlog - 4

## Table of Contents
1. [Back to the Porture](#41---back-to-the-porture)
	- [Raise Again, Green Snake](#411-raise-again-green-snake)
	- [Yellow Snake, Please Go to Aisle Game](#412-yellow-snake-please-go-to-aisle-game)
	- [Re-Diced and Re-Sliced](#413-re-diced-and-re-sliced)
	- [Post-Post-Processing](#414-post-post-processings)
	- [Why the Long Face?](#415-why-the-long-face)
	- [DAYLIGHT](#416-daylight-or-pánico-en-el-túnel-starring-silvester-stallone)
	- [Do You Want to See the Menu?](#417-do-you-want-to-see-the-menu)
2. [The Current State of Main Things (or The Main State of Current Things)](#42---the-current-state-of-main-things-or-the-main-state-of-current-things)

<br>
<br>


# 4.1 - Back to the Porture
As stated at the end of the third log, it's time to go back to porting systems, as well as some other stuff that is still pending in the already ported one (like the 3D rendering pipeline, for example). Let's start with recovering the `AI`, which is going to need for a new `AIComponent`, some helpers and, of course, an `AISystem`. And because we're now all json-y, we'll take what were code-stated AI configurations into an `AIPresets.json` data file, which will need an `AIPresetLoader`. Nothing new, same old.

<br>

### 4.1.1 Raise Again, Green Snake
Let's get into the AI pipeline from what is, now and forever, the usual ground stone: the component. We need an `AIComponent`, which is going to be similar as the data struct in the json file:
```cpp
#pragma once

#include <vector>
#include "../../incs/DataStructs.hpp"

struct AIComponent {
	std::vector<Vec2>   path;
	Vec2                target = { 0, 0 };

	AIBehaviourState    behavior            = AIBehaviourState::MEDIUM;
	int                 maxSearchDepth      = 150;
	bool                useSafetyCheck      = true;
	bool                hasSurvivalMode     = true;
	bool                predictOpponent     = false;
	float               randomMoveChance    = 0.0f;
	float               aggresiveness       = 0.5f;
};
```
Next, we'll have to handle the loading of data to build a new json → struct pipeline. A combination of parsing, file managing and json processing functions should do the trick, in a similar fashion as what was put in place for the `CollisionRule`s. Here's the concoction:
```cpp
using json = nlohmann::json;

static AIBehaviourState parseBehavior(const std::string& s) {
	if (s == "EASY")   return AIBehaviourState::EASY;
	if (s == "MEDIUM") return AIBehaviourState::MEDIUM;
	if (s == "HARD")   return AIBehaviourState::HARD;
	throw std::runtime_error("AIPresetLoader: unknown behavior: " + s);
}

AIPresetLoader::PresetTable AIPresetLoader::load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("AIPresetLoader: cannot open file: " + path);

	json data;
	try {
		file >> data;
	} catch (const json::parse_error& e) {
		throw std::runtime_error("AIPresetLoader: JSON parse error: " + std::string(e.what()));
	}

	PresetTable table;

	for (const auto& entry : data.at("AIPresets")) {
		AIComponent preset;
		preset.behavior         = parseBehavior(entry.at("behavior").get<std::string>());
		preset.maxSearchDepth   = entry.at("maxSearchDepth").get<int>();
		preset.useSafetyCheck   = entry.at("useSafetyCheck").get<bool>();
		preset.hasSurvivalMode  = entry.at("hasSurvivalMode").get<bool>();
		preset.randomMoveChance = entry.at("randomMoveChance").get<float>();
		preset.aggresiveness    = entry.at("aggressiveness").get<float>();

		const std::string name = entry.at("name").get<std::string>();
		table[name] = preset;
	}

	return table;
}
```

All of this needs, of course, the json base data, which has entries like the following:
```json
{
	"name":             "easy",
	"behavior":         "EASY",
	"maxSearchDepth":   50,
	"useSafetyCheck":   false,
	"hasSurvivalMode":  false,
	"randomMoveChance": 0.15,
	"aggressiveness":   0.8
},
```

And with all the already existing (taken from the OOP version, adapted) helpers contained in `AITypes`, `FloodFill`, `PathFinder` and `GridHelper`, we're all set for the closing part, the `AISystem`:
```cpp
using BlockedGrid = std::vector<std::vector<bool>>;

class AISystem {
	private:
		int         _gridWidth;
		int         _gridHeight;
		Pathfinder  _pathFinder;
		FloodFill   _floodFill;
		GridHelper  _gridHelper;

		BlockedGrid buildBlockedGrid(Registry& registry, const ArenaGrid* arena) const;

		Direction decideDirection(Registry& registry, Entity entity, const BlockedGrid& blocked);
		Direction goToFood      (Registry& registry, Entity entity, const BlockedGrid& blocked);
		Direction survivalMode  (Registry& registry, Entity entity, const BlockedGrid& blocked);
		Direction maximizeSpace (Registry& registry, Entity entity, const BlockedGrid& blocked);

		bool      isSafeMove    (const BlockedGrid& blocked, Vec2 nextPos) const;
		Direction vecToDirection(Vec2 from, Vec2 to) const;
		Vec2      stepInDirection(Vec2 pos, Direction dir) const;

	public:
		AISystem(int gridWidth, int gridHeight)
			: _gridWidth(gridWidth), _gridHeight(gridHeight) {}

		// arena may be nullptr, falls back to SolidTag scan
		void update(Registry& registry, const ArenaGrid* arena = nullptr);
};
```

As can be seen in the snippet, the system has a regular `update()` public interface which works in this order:
1. Get a layout of the blocked cells in the grid
2. Make a direction decision
3. Set the AI snake's `MovementComponent`'s direction to the decided one

All of the steps in this process are a mixture of pathfinding, floodfilling, walkable checks and so on. Nothing new with respects to what the OOP version's `AI` pipeline was already achieving, just adapted and refined to work in the new ECS/DD context. Some AI snake factorization here, some `AISystem` instantiation there, a precise `update()` call in the game loop... And once again, it's alive!!

<br>

### 4.1.2 Yellow Snake, Please Go to Aisle Game
Before moving on to the next system of choice, there are a couple of things related to already ported systems that I would like to recover in this new build: the **1v1 multiplayer** mode and the **3D rendering pipeline**. We'll tackle the former first, so that we can resurrect our WASD yellow snake. If my (usually wrong by miles) calculations are correct, this should be extremely trivial, just a secon factory call to create a second snake entity with an associated input slot of `B`. And... It was easy, yeah, but during the implementation a found a tasty bug.

See, I had to slightly tweak the `spawnPlayerSnake()` factory function to mirror the moving directions when having a 2 snake initial set up (be it `VSAI` or `MULTI`). I added a direction variable definition before pushing components into the registry, but because I had the segment creation BEFORE this steps, i ended up having a pre-defined, always-towards-right initial segment setup that clashed with the direction preset selected afterwards. This translated in a snakes that were always moving one cell to the right in the first movement tick, then changing into the set directions. Which, in itself, meant that any snake spawned with a LEFT direction automatically collied with itself. The huntdown of this bug took some time, but luckily the fix was pretty easy, just had to move the segment initialization to be after the direction definition:
```cpp
Entity Factories::spawnPlayerSnake(Registry& registry,
								InputSystem& inputSystem,
								Vec2 startPos,
								int initialLength,
								BaseColor color,
								PlayerSlot slot) {
	Entity e = registry.createEntity();
	SnakeComponent snake;

	Direction direction = (registry.view<SnakeComponent>().empty()) ? Direction::RIGHT : Direction::LEFT;

	for (int i = 0; i < initialLength; ++i) {
		Vec2 segPos;
		switch (direction) {
			case Direction::RIGHT: segPos = { startPos.x - i, startPos.y }; break;
			case Direction::LEFT:  segPos = { startPos.x + i, startPos.y }; break;
			case Direction::DOWN:  segPos = { startPos.x, startPos.y - i }; break;
			case Direction::UP:    segPos = { startPos.x, startPos.y + i }; break;
		}
		snake.segments.push_back({ segPos, BeadType::None });
	}

	registry.addComponent(e, snake);
	registry.addComponent(e, PositionComponent{ startPos });
	registry.addComponent(e, MovementComponent{ direction, 0.0f, 0.1f });
	registry.addComponent(e, InputComponent{});
	registry.addComponent(e, RenderComponent{ color });
	registry.addComponent(e, ScoreComponent{});
	inputSystem.assignSlot(e, slot);
	return e;
} 
```
> *On a side note, the way the snake spawning factory function knows if it's dealing with a first or a second snake is by checking if the registry's `SnakeComponent` pool is empty. If it is, first snake. If it's not, second snake. Method works after the fact that, as of today, there will never be more than 2 snakes. If that fact changes, this function shall too.*

> *On another sidenote, I added a `GameMode` enum in `DataStructs.hpp`, which is now sent to the `reset()` function in `GameState` for it to know if the (re)initialization of the game needs one or two snakes, and what type of snake if the latter is the case.*

<br>

### 4.1.3 Re-Diced and Re-Sliced
Having ported the `2D` rendering pipeline and having the OOP version's `3D` rendering functions in the rearview mirror, recovering the `3D` drawings in the new structure is easy peasy. The process mimics the registry query of the `2D` version and builds `Vector3` based positions to draw the checkerboard ground, the snakes and the food pip. There are no walls here, nor is there implemented a obstacle translation from the `2D` side of the game, as that is a future implementation, past the current porting phase. This process is quite unimportant, and one of the new `3D` drawing functions suffices as an illustration:
```cpp
void RenderSystem::drawSnakes3D(Registry& registry) const {
	for (auto entity : registry.view<SnakeComponent, RenderComponent>()) {
		const auto& snake = registry.getComponent<SnakeComponent>(entity);
		const auto& render = registry.getComponent<RenderComponent>(entity);

		float offsetX = (_gridWidth * _cubeSize) / 2.0f;
		float offsetZ = (_gridHeight * _cubeSize) / 2.0f;

		for (size_t i = 0; i < snake.segments.size(); i++) {
			Vector3 position = {
				snake.segments[i].position.x * _cubeSize - offsetX,
				_cubeSize,
				snake.segments[i].position.y * _cubeSize - offsetZ
			};

			float size = (i == 0) ? _cubeSize : _cubeSize * 0.8f;
			if (i > 0) position.y  *= 0.8f;
			
			if (i % 2 == 0) {
				drawCubeCustomFaces(position, size, size, size, snakeALightFront, snakeAHidden, snakeALightTop, snakeAHidden, snakeALightSide, snakeAHidden);
			} else {
				drawCubeCustomFaces(position, size, size, size, snakeADarkFront, snakeAHidden, snakeADarkTop, snakeAHidden, snakeADarkSide, snakeAHidden);
			}
		}
	}
}
```

What actually is more interesting is... A new B U G emerged! Nothing related to the `3D`, really, just something I noticed while testing it. The error triggered if a player's inputs were too fast and contained a 180 degree reversal. For example, with a player controlled right bound snake, if its owner quickly input `UP` and `LEFT`, say at the same time, it could be the case that the two inputs be processed before a movement tick, resulting in a bypassing of the 180 degree guard. In other words, before the snake moved one position (cell/cube), `UP` and then `LEFT` were processed, and because `LEFT` was processed after direction was switched to `UP`, the guard failed, and when the movement tick arrived, the snake went from `RIGHT` to `LEFT`, colliding into itself. The solution to this was twofold:
- Call `processInput` inside `advanceSnake`, right before the movement happens and **consuming input only at move tick time**
- **Make Reversal check use `lastDirection` (the direction AT THE MOMENT of movement), instead of `move.direction` (which may have already been updated by a previous buffered input)**.
	- The `break` after accepting a valid direction also ensures only **one** direction change is applied per tick, discarding the rest of the buffer.
```cpp
// reads each entity's InputComponent and updates its Movement Component direction.
// ignores inputs that would reverse the current direction and discards invalid buffers (silently)
// lastDirection is the direction the snake last actually moved (not the buffered one)
void MovementSystem::processInput(Registry& registry, Direction lastDirection, MovementComponent& move, InputComponent& input) {
	while (!input.inputBuffer.empty()) {
		const Input next = input.inputBuffer.front();
		input.inputBuffer.pop();

		Direction requested = move.direction;

		switch (next) {
			case Input::Up_A:    requested = Direction::UP;    break;
			case Input::Down_A:  requested = Direction::DOWN;  break;
			case Input::Left_A:  requested = Direction::LEFT;  break;
			case Input::Right_A: requested = Direction::RIGHT; break;
			case Input::Up_B:    requested = Direction::UP;    break;
			case Input::Down_B:  requested = Direction::DOWN;  break;
			case Input::Left_B:  requested = Direction::LEFT;  break;
			case Input::Right_B: requested = Direction::RIGHT; break;
			default: break;
		}

		const Vec2 lastVec      = directionToVec2(lastDirection);
		const Vec2 requestedVec = directionToVec2(requested);

		bool isReversal = (lastVec.x + requestedVec.x == 0) &&
						(lastVec.y + requestedVec.y == 0);

		if (!isReversal) {
			move.direction = requested;
			break; // only apply one valid direction per move tick
		}
	}
}
```
```cpp
// advances the move timer for each snake entity.
// when timer exceeds interval, consumes input and moves one grid cell
void MovementSystem::advanceSnake(Registry& registry, float deltaTime) {
	for (auto entity : registry.view<MovementComponent, PositionComponent, SnakeComponent>()) {
		auto& move	= registry.getComponent<MovementComponent>(entity);
		auto& pos	= registry.getComponent<PositionComponent>(entity);
		auto& snake	= registry.getComponent<SnakeComponent>(entity);

		move.moveTimer += deltaTime;
		if (move.moveTimer < move.moveInterval)
			continue;
		move.moveTimer = 0.0f;

		if (snake.segments.empty())
			continue;

		// consume input only when the snake is about to move,
		// using the last committed direction as the reversal reference
		if (registry.hasComponent<InputComponent>(entity)) {
			auto& input = registry.getComponent<InputComponent>(entity);
			processInput(registry, move.direction, move, input);
		}

		const Vec2 delta	= directionToVec2(move.direction);
		const Vec2 newHead	= { snake.segments.front().position.x + delta.x,
								snake.segments.front().position.y + delta.y };

		snake.segments.push_front({ newHead, BeadType::None });
		pos.position = newHead;

		if (snake.growing) {
			snake.growing = false;
		} else {
			snake.segments.pop_back();
		}
	}
}
```

And with this, it appears that the snake is no longer killing itself, which is good news.

<br>

### 4.1.4 Post-Post-Processings
This surely is the most easy system to port, as it was never really affecting any object in the game, but just expanding the rendering pipeline so that the `RenderSystem` (formerly `Renderer`) drew into a screen texture, then applying some shader-based effects on it. The new version is practically a 1:1 image of the OOP's one, just with an added `shudtdown()` function to clean up the GPU related assets, to be called before closing the `Raylib` window. The new `PostProcessingSystem` just needs to be instantiated in `main` and initialized, after which a small change in the game loop's rendering phase is introduced. Instead of a straight up, `RenderSystem` based `BeginDraw()` call, we recovered the `PostProcessingSystem` based `beginCapture()`-`endCapture()` detour, which results in, well, fancy visuals again!
```cpp
while (true) {
		if (WindowShouldClose()) break;

		DrawFPS(SCREEN_W - 95, 10);

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

		if (IsKeyPressed(KEY_F)) ToggleFullscreen();
		if (IsKeyPressed(KEY_ONE)) renderMode = RenderMode::MODE2D;
		if (IsKeyPressed(KEY_TWO)) renderMode = RenderMode::MODE3D;
		if (IsKeyPressed(KEY_P)) postProcessingSystem.togglePostprocessing();

		// fresh context each frame
		FrameContext ctx;
		ctx.arena       = &arena;
		ctx.gridWidth   = GRID_W;
		ctx.gridHeight  = GRID_H;
		ctx.renderMode  = &renderMode;
		ctx.playerDied  = false;
		
		// update phase
		inputSystem.update(registry);
		aiSystem.update(registry, ctx);
		movementSystem.update(registry, dt);
		collisionSystem.update(registry, ruleTable, dispatcher, ctx);

		if (ctx.playerDied) {
			std::cout << "PLAYER DIED" << std::endl;
			break;
		}

		// render phase
		postProcessingSystem.beginCapture();
		renderSystem.render(registry, dt, ctx);
		postProcessingSystem.endCapture();

		// post processing phase
		BeginDrawing();
		ClearBackground(customBlack);
		postProcessingSystem.applyAndPresent(dt);
		EndDrawing();
	}
```

As you can see, I also added a key hook (`P`) to toggle the whole post processing. This shall be tied to an options menu section down the line, but for now is functional and serves a developing purpose.

> *I need to figure out if this straight key hooks in the `main` loop are correctly placed in their current locations, of I should take them to a system based management (maybe the already existing `InputSystem`)*.

<br>

### 4.1.5 Why the Long Face?
*Because you don't look very animated hahahahah*. Time to tackle what seems, from a distance, the most troublesome system to port and resucitate the animation pipeline. As has been the protocolary approach, the first thing when building (porting, really) a system is to ask ourselves: **what does the animation system actually need to store and communicate?**. Looking at the OOP's `AnimationSystem` and `ParticleSystem`, it really appears as there is not going to be much need for big changes, as they were quite stand-alone systems to begin with, but a good thinking angle is comprised of the **distinct animated objects** managed by these systems, and what their natures are:
1. **Tunnel Lines**: No entity ownership.
	- `TunnelLine` and `TunnelConfig` are **purely internal to the system**. No entity in game *is* a tunnel line, no entity *owns* one. This means **no new component needed**: the new system will own its data entirely, just as it did in OOP.
2. **Particles**: Spawn triggers need a bridge
	- Particles themselves are also internal to `ParticleSystem`, but *spawning* them is triggered by game events that live in ECS land:
		- **Trail** → triggered by snake movement (i.e., a `SnakeComponent` entity moving).
		- **Explosion** → triggered by food being eaten (i.e, a collision effect)
		- **Dust** → purely internal timer, no entity involved.

So, **the trail and explosion cases are the only ones that need to cross the ECS boundary**. The urgent shift in mindset at this point, if pending, is to engrave in our brains that an **entity** (and it's attached components) is something that *lives* in the game, not just something that *is* in the game. `snakes` and `food` are game-tied entities, the things that move and collide and sustain the mechanics and dynamics of the whole loop, but things like `particles`, `tunnelLines` and even `walls` and `obstacles` exist as context, visual flair or plain constraints. At least for now, as the game's design still has a long, long way to go.

Anyway, back to the proting process, the first step seems clear: we need to **decide how spawn events cross from ECS into `ParticleSystem`**. And for this we have two main, clean options regarding consistency with what's already built:

#### Option a → `ParticleSpawnRequest` component
```cpp
struct ParticleSpawnRequest {
	enum class Type { Trail, Explosion };
	Type    type;
	float   x, y;
	int     count;
	float   direction; // for trails
	Color   color;
};
```

This is a short-lived component, mirroring how `CollisionEffects` works, consumed and removed by `ParticleSystem` each frame. Placed on an entity by `CollisionEffects` (explosion) or `RenderSystem` (trail), taken care of by the `update()` function in `ParticleSystem`.

#### Option b → Direct calls from `RenderSystem`
This would mean no component at all, therefore simpler. `RenderSystem` already queries snake positions to draw them, so it can just also call `ParticleSystem.spawnSnakeTrail(...)` directly in the same loop. BUT, explosions would need `CollisionEffects` to hold a pointer/reference to `ParticleSystem`, which is less clean, and would totally go against what we have been attempting (and, to the best of my knowledge, achieving), in these new pure ECS rebuild.

Obviously, we're going with **option a** because of consistency. We'll keep `CollisionEffects` free of system pointers, and the component will act as a pure data message, the same pattern as what's in place via `FrameContext` to pass data between systems. So let's take the component from just above and build the new `ParticleSystem` around it.

A handfull of things need to be set up, following the creation of a new subdirectory at `srcs/particles/`: `ParticleConfig` header and json data file, `ParticleConfigLoader` and the `ParticleSystem` itself. Regarding the first ones, nothing remarkable nor new, we just need to put in place an external configuration file with data for all the currently implemented particles, a struct container in the code side and a dedicated loader to parse the json into the inner struct. With that out of the way, the new `ParticleSystem`'s header looks like this:
```cpp
#pragma once

#include <vector>
#include <iostream>
#include <raylib.h>
#include "ParticleConfig.hpp"
#include "../ecs/Registry.hpp"
#include "../components/ParticleSpawnRequest.hpp"

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

class ParticleConfigLoader {
public:
	static ParticleConfig load(const std::string& path);
};

class ParticleSystem {
	std::vector<Particle>   _particles;
	ParticleConfig          _config;

	int     _screenWidth;
	int     _screenHeight;
	float   _dustSpawnTimer = 0.0f;

	// internal spawn helpers
	void    _spawnDust();
	void    _spawnExplosion(float x, float y);
	void    _spawnTrail(float x, float y, float direction, Color color);

	// render helper
	void    _drawRotatedSquare(float cx, float cy, float size,
							float rotation, Color color, unsigned char alpha) const;

public:
	ParticleSystem(int screenW, int screenH, ParticleConfig config);

	void    update(float dt, Registry& registry);   // consumes ParticleSpawnRequests
	void    render() const;

	void    clear();
	size_t  getParticleCount() const;
};
```

Before divind deep into the implementation `cpp` file, though, a very important question needs to be cleared, affecting `update()` and `_spawnDust()`. In the OOp version, `spawnDustParticle()` picked a random position within the grid bounds (using `cellSize` and `borderOffset` to map to screen space). Because those are now removed from the new system since requesters already work in screen space, and regarding dust (purely internal with no requester), **where should it spawn?**. Once again, we find ourselves with a forking possibility:
- **Full Screen**: just pick a random position anywhere in `[0, _screenWidth] x [0, _screenHeight]`
- ** Bounded to current arena**: would require passing arena bounds into Particle System, which was explictly avoided.

Aaaaand... The answer ended up being a third option: **bounded to the arena, via `FrameContext`**. The `FrameContext` struct, already flowing through every system per frame, was extended with `arenaBounds`, `cellSize`, `gameAreaX` and `gameAreaY`, which are populated by `RenderSystem::fillContext()` before the update phase. This means `ParticleSystem` doesn't hold a pointer to anything it shouldn't, and doesn't need to know what an `ArenaGrid` is. It just reads layout data from a struct that was always going to pass through it anyway. C L E A N.

One additional refinement on top of this: dust is constricted to the game arena, with an inset calculation so it never spawns on top of the wall border. The playfield bounds passed to `spawnDust()` are computed from `ctx.gameAreaX/Y` + `ctx.cellSize` rather than from the raw `arenaBounds`, which includes the border thickness:
```cpp
const float cs = static_cast<float>(ctx.cellSize);
ArenaBounds playfield {
	ctx.gameAreaX + cs,
	ctx.gameAreaY + cs,
	static_cast<float>(ctx.gridWidth  - 2) * cs,
	static_cast<float>(ctx.gridHeight - 2) * cs
};
spawnDust(playfield);
```

With that settled, the `update()` function does four things in order: consume `ParticleSpawnRequest` components from the registry (dispatching to the appropriate spawn helper per type), run the ambient dust timer, simulate all live particles (integrate position, apply drag, rotate, shrink toward end-of-life), and erase dead ones. The `render()` function is a flat loop that draws each particle as a rotated rectangle via `DrawRectanglePro`, fading alpha with the remaining life ratio. No state, no retained draw calls, just iterate and draw.

> *You might think that this doubles the `render()` functions, but the decoupling of regular rendered stuff and particles is necessary so that each system stays independent, isolated and scalable*

On the spawning side, the two ECS-triggered types each have their own path:
- **Explosion**: placed by `CollisionEffects::RelocateFood()` with `gridCoords = true`, so the `ParticleSystem` converts the grid position to screen space on consumption.
- **Trail**: placed by `RenderSystem::drawSnakes2D()`, already inside the screen-space loop, with `gridCoords = false`, so the spawn position arrives ready to use.

Dust doesn't cross the ECS boundary at all. No entity owns it, no component triggers it. The timer ticks inside `ParticleSystem` and that's the end of the story. The render phase now looks like this:
```cpp
// render phase
postProcessingSystem.beginCapture();
if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D)
	particleSystem.render();
renderSystem.render(registry, dt, ctx);
postProcessingSystem.endCapture();
```

One final detail worth noting: trail density and spread were initially coupled, since `count` controlled both. A `trailSpawnInterval` field was added to `ParticleConfig`, throttling how often trail emission fires independently of how many particles each emission bursts out. `count` governs spread; `spawnInterval` governs density. Both are JSON-tunable without touching a line of code:
```json
"trail": {
	"count":         2,
	"scatter":       15.0,
	"spawnInterval": 0.05
}
```

And with that, the animation pipeline is back on its feet. Three particle types, two ECS-crossed spawn paths, one timer-driven ambient system, and zero pointers to things that shouldn't be pointed at. It's time to go back to the T U N N E L.

<br>

### 4.1.6 DAYLIGHT (or *Pánico en el túnel* starring Silvester Stallone)

The OOP version's `AnimationSystem` drew concentric outlines of the arena shape that expanded outward from the screen center on a continuous spawn interval, giving the background a slow, pulsing depth. It was entirely self-contained and had no entity in the ECS sense, which means the porting question here is simpler than it was for particles: **no new component needed at all**. The system owns its data start to finish. What it does need is:
- A way to know the arena's outline geometry.
- A way to know where the arena sits on screen.
- A way to know when the arena changes shape, so in-flight lines keep rendering the *old* outline while new lines spawn following the *new* one.

> *Thankfully, the bulk of this system's hard work was done in the OOP version, and given that the port is going to be pretty straight forward, this is going to go down easily*

#### The geometry source: `getAllOutlines`

`ArenaGrid::getAllOutlines(offsetX, offsetY)` already existed from the OOP build. It walks the grid and, for every contiguous block of solid cells, emits a polygon representing its outer boundary as a `vector<Vector2>`. The signature takes an offset, but what it actually returns are raw grid-unit coordinates in the form `offsetX + cellIndex`. In other words, not screen pixels, integer grid indices offset by a pixel value, with no cell size scaling applied. That detail became relevant later.

#### `TunnelLine` and `TunnelConfig`

The internal data structures are a direct port:

```cpp
struct TunnelConfig {
	int     borderThickness = 15;
	int     contentInset    = 40;
	float   spawnInterval   = 0.15f;
	float   animationSpeed  = 0.7f;
	int     maxLines        = 12;
	Color   lineColor       = { 144, 238, 144, 255 };
	float   lineThickness   = 2.0f;
};

struct TunnelLine {
	float   progress = 0.0f;   // 0 = at center (just spawned), 1 = at edge (dead)
	float   age      = 0.0f;
	int     epoch    = 0;
};
```

`TunnelLine::epoch` is the arena shape generation the line was spawned into. When the arena changes, the epoch counter is incremented, and all live lines are stamped with the *old* epoch. The render pass selects either `_previousShapes` or `_currentShapes` based on whether the line's epoch matches the current one. This is the mechanism that lets old lines finish their animation against the old outline while new lines follow the new one, a clean dual-epoch scheme (which is logged in detail in past documents).

#### The epoch system and `notifyArenaSpawning`

When the arena is about to change, `main` calls `animationSystem.notifyArenaSpawning(arena)` before applying the new preset. This:
1. Snapshots `_currentShapes` into `_previousShapes`.
2. Increments `_currentEpoch`.
3. Stamps all live lines with `_currentEpoch - 1` (the old generation).
4. Recomputes `_currentShapes` from the arena's *new* outline (even though the arena hasn't been transformed yet, this anticipates the upcoming change).

This means the moment `transformArenaWithPreset()` fires, the animation system is already aligned with what comes next. Old lines will keep rendering against `_previousShapes` until they fade out at `progress >= 1.0`. New lines pick up `_currentShapes` immediately. The `notifyArenaDespawning` variant does the same thing but also sets `despawnPending = true`, which tells `update()` to watch for all old-epoch lines to die before firing the `onDespawnReady` callback. This is how a future menu or transition layer can wait for the visual drain to complete before proceeding.

#### `update` and rendering

`update(dt, arena)` runs every frame: it ages and eases all live lines, removes completed ones, manages the `despawnPending` drain, ticks the spawn interval, and caches `_currentShapes` from the arena's current state. The easing used is `easeInQuad`: lines accelerate as they expand outward, which gives the animation a slightly more organic, exhaling feel compared to a linear slide.

`render()` iterates every live line, selects the shape set matching its epoch, and for each shape polygon calls `renderLine()`. That function computes an inset version of the polygon toward the screen center by interpolating between the outer shape and the center point, using the line's `progress` as the interpolation driver (inverted; at `progress = 0`, fully inset; at `progress = 1`, at the outer edge). The result is a wireframe that expands from the center outward and fades in alpha as it goes.

#### The coordinate scaling bug

Initial testing produced tunnel lines rendered as a tiny cluster of pixel-sized shapes crammed into the top-left corner. The cause was the `getAllOutlines` coordinate space issue described above: the function returns `offsetX + cellIndex`, where `cellIndex` is a raw grid integer (e.g. `0`, `1`, `2`...), not a pixel position. Feeding that directly into screen-space draw calls meant drawing at positions like `(336 + 0, 208 + 1)` instead of `(336 + 0*32, 208 + 1*32)`. The fix was a `scaleOutlines()` helper that applies the cell size scaling after the fact:
```cpp
std::vector<std::vector<Vector2>> AnimationSystem::scaleOutlines(std::vector<std::vector<Vector2>> raw) const {
	const float ox = static_cast<float>(_offsetX);
	const float oy = static_cast<float>(_offsetY);
	const float cs = static_cast<float>(_cellSize);

	for (auto& shape : raw)
		for (auto& p : shape) {
			p.x = ox + (p.x - ox) * cs;
			p.y = oy + (p.y - oy) * cs;
		}
	return raw;
}
```

Every outline coming out of `getAllOutlines` is passed through `scaleOutlines` before being stored in `_currentShapes` or `_previousShapes`. After this fix, the lines correctly traced the full arena border at screen scale.

#### Initialization and screen layout

`AnimationSystem::init()` takes the screen dimensions and the arena's pixel offset and cell size. These values aren't easily available at construction time (they depend on `RenderSystem::fillContext()`, which needs an active window), so a throwaway `fillContext()` call in `main` bridges the gap before the game loop starts:

```cpp
{
	ArenaGrid tmpArena(GRID_W, GRID_H);
	FrameContext tmpCtx;
	tmpCtx.arena = &tmpArena;
	tmpCtx.gridWidth = GRID_W; tmpCtx.gridHeight = GRID_H;
	renderSystem.fillContext(tmpCtx);
	animationSystem.init(SCREEN_W, SCREEN_H,
		static_cast<int>(tmpCtx.arenaBounds.x),
		static_cast<int>(tmpCtx.arenaBounds.y),
		tmpCtx.cellSize);
}
animationSystem.enable(true, tunnelPresets.at("realm2D"));
```

The temporary arena is immediately discarded; its sole purpose is to let `fillContext` compute the layout math so `init` gets real numbers.

#### Making `TunnelConfig` data-driven

Consistent with the rest of the build, `TunnelConfig` is no longer hardcoded. A `data/TunnelConfig.json` holds named presets:

```json
{ "presets": [
	{ "name": "realm2D", "borderThickness": 15, "contentInset": 40,
	"spawnInterval": 0.15, "animationSpeed": 0.7, "maxLines": 12,
	"lineColor": [70, 130, 180, 255], "lineThickness": 2.0 },
	{ "name": "menu", ... }
]}
```

`TunnelConfigLoader::load()` follows the same static-load-into-map pattern established by the previous loaders, producing a `PresetTable` (i.e. `unordered_map<string, TunnelConfig>`) keyed by preset name. `main` picks the active preset by name at startup.

#### Arena presets and `KEY_TAB` cycling

To actually exercise the epoch transition pipeline, old lines draining while new ones spawn against a changed outline, we needed a way to cycle arena layouts at runtime. `data/ArenaPresets.json` holds the 10 available `WallPreset` entries by name:

```json
{ "presets": [
	{ "name": "InterLock1" }, { "name": "Spiral1" }, { "name": "Columns1" },
	{ "name": "Columns2"  }, { "name": "Cross"    }, { "name": "Checkerboard" },
	{ "name": "Maze"      }, { "name": "Diamond"  }, { "name": "Tunnels" },
	{ "name": "FourRooms" }
]}
```

`ArenaPresetLoader::load()` maps name strings to `WallPreset` enum values and returns an ordered `vector<WallPreset>` (order matters for cycling). In `main`, `KEY_TAB` advances through the list:

```cpp
if (IsKeyPressed(KEY_TAB)) {
	currentPresetIndex = (currentPresetIndex + 1) % static_cast<int>(arenaPresetList.size());
	animationSystem.notifyArenaSpawning(arena);
	arena.transformArenaWithPreset(arenaPresetList[currentPresetIndex]);
	float lineLifetime = 1.0f / animationSystem.getAnimationSpeed();
	arena.beginSpawn(lineLifetime);
}
```

The `notifyArenaSpawning` call goes first — it snapshots the current shapes and stamps live lines with the old epoch before the geometry changes. `transformArenaWithPreset` then applies the new layout, and `beginSpawn` kicks off the arena cell fade-in animation. From the animation system's point of view, the transition is seamless: existing lines finish their path against the old outline, new lines immediately begin spawning along the new one.

> *This tab-based cycling through presets is just temporary, something to test the pipelines. The arena changes will be hooked up to gameplay events later on*

#### Draw order

Tunnel lines sit behind everything else in the 2D render pass. The order inside `BeginMode2D`:

```
animationSystem.render()    // tunnel lines: deepest layer
particleSystem.render()     // particles: above lines, below game objects
renderSystem.render2D()     // arena, food, snakes: on top
```

This ensures lines never occlude gameplay elements, which would be particularly distracting given they expand continuously across the full playfield.

<br>

### 4.1.7 Do You Want to See the Menu?

The menu system poses an interesting architectural challenge. In the OOP version, menus were a state handler with callbacks attached to buttons: click a button, it calls a lambda that changes game state. In an ECS world, that pattern violates the principle of decoupled systems: a UI button shouldn't know about (or hold a reference to) a callback (imagine learning how to set up callbacks just to then figure out that they go against that complex paradigm you're trying to master, so funny). Instead, we need the button system to be a first-class ECS inhabitant, using the same patterns we've relied on everywhere else: **components for data, systems for behavior, and an event queue to decouple concerns**.

The new, ECS focused menu system now has four key pieces:

1. **Button Components**: Data structures defining button state
2. **EventQueue**: A message bus for button actions (*the need for an event bus was long overdue*)
3. **ButtonFactory**: Entity spawning from JSON config
4. **UIInteractionSystem**: Input handling and event emission

So let's go through the new design from start to finish, little by little, step by step, *al pasito*.

#### 4.1.7.1 Button as ECS Entity

A button starts as data, just as every other inhabitant of the game. Four new components needed to be defined to completely built the notion of a button in our new ECS paradigm:

```cpp
// ButtonComponent: the core button state
struct ButtonComponent {
	std::string          id;           // "start_button", "mode_button", etc
	Rectangle            bounds;       // screen-space collision rect
	ButtonConfig         config;       // visual styling from JSON
	bool                 hovered;      // updated by UIInteractionSystem
};

// UIRenderableComponent: marker tag so that entities can be rendered by MenuSystem
struct UIRenderableComponent {};

// UIInteractableComponent: tag so that entities can process mouse input in UIInteractionSystem
struct UIInteractableComponent {};

// ButtonActionComponent: what actually happens when a button is clicked
struct ButtonActionComponent {
	enum class ActionType {
		StartGame,
		ChangeMode,
		Quit,
		ReturnToMenu
	};
	ActionType action;
};
```

Considered all together, we can say that the first three provide the button **data**, while the last one carries the button's **behavior type**. Summing up: yeah, buttons are now entities. They can be spawned, destroyed, queried, iteretaed through... Same as any other entity in the build: just ask the `Registry` and work with components!

#### 4.1.7.2 Buttons from (press X to)JSON(!!!!)

Just like collision rules, AI presets, and particle configs, button definitions now live in `data/ButtonConfig.json`:

```json
{
	"menus": {
		"start": [
			{
				"id":           "start_button",
				"label":        "START",
				"fontSize":     40,
				"backgroundColor": [30, 30, 30, 255],
				"hoverColor":      [60, 60, 60, 255],
				...
			},
			{
				"id":           "mode_button",
				"label":        "MODE: SINGLE",
				"fontSize":     32,
				...
			}
		],
		"gameover": [
			{
				"id":           "retry_button",
				"label":        "RETRY",
				...
			},
			...
		]
	}
}
```

A `ButtonConfigLoader` parses this into:

```cpp
class ButtonConfigLoader {
public:
	struct ButtonTable {
		std::map<std::string, std::vector<ButtonConfig>> menus;
	};
	static ButtonTable load(const std::string& path);
};
```

All button visual properties (colors, sizes, positions) are loaded from JSON. The system doesn't hardcode a single value. ECSing!!!

#### 4.1.7.3 ButtonFactory: declarative spawning

Instead of `MenuSystem` manually creating button objects and wiring them up, a very OOP way (EUGH) of doing things, we now use a factory:

```cpp
class ButtonFactory {
public:
	static std::vector<Entity> spawnButtonsFromConfig(
		Registry& registry,
		const ButtonConfigLoader::MenuButtonTable& config,
		int screenWidth, int screenHeight
	);
};
```

This function:
1. Iterates the button config table
2. Creates an entity for each button
3. Attaches `ButtonComponent` with position, size, and styling
4. Attaches marker components (`UIRenderableComponent`, `UIInteractableComponent`)
5. Attaches `ButtonActionComponent` with the action enum type
6. Returns a vector of the spawned entity IDs (so `MenuSystem` can track which buttons to render)

The factory handles all the layout math, like centering buttons, spacing them, and converting grid coordinates to screen pixels. `MenuSystem` doesn't touch any of that. Because we're decoupled to the M A X.

#### 4.1.7.4 UIInteractionSystem: Menu Input Handling (or: Mousing!)

A new system processes mouse input:

```cpp
class UIInteractionSystem {
public:
	void update(Registry& registry, EventQueue& eventQueue);
};
```

This system:
1. Queries for entities with `ButtonComponent` + `UIInteractableComponent` + `ButtonActionComponent`
2. Gets the mouse position via Raylib
3. For each button, checks if the mouse is inside its bounds → updates `hovered` state
4. Checks for a mouse click
5. If clicked, enqueues a `GameEvent` to the event queue

The core logic here keeps being simple, based around a button lookup, which is *acceptable* in the current build, were there is a small amount of buttons, i.e. the overhead is not important. It looks like this:

```cpp
void UIInteractionSystem::update(Registry& registry, EventQueue& eventQueue) {
	Vector2 mousePos = GetMousePosition();
	bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT);
	static bool clickHandled = false;  // Prevent multi-button clicks
	
	auto buttonView = registry.view<ButtonComponent, UIInteractableComponent, ButtonActionComponent>();
	
	for (Entity entity : buttonView) {
		auto& button = registry.getComponent<ButtonComponent>(entity);
		auto& action = registry.getComponent<ButtonActionComponent>(entity);
		
		// Hover detection
		button.hovered = CheckCollisionPointRec(mousePos, button.bounds);
		
		// Click detection
		if (button.hovered && mouseClicked && !clickHandled) {
			eventQueue.enqueue({
				GameEvent::Type::ButtonClicked,
				action.action
			});
			clickHandled = true;
		}
	}
	
	if (!mouseClicked) clickHandled = false;  // Reset next frame
}
```

Notice: `UIInteractionSystem` has **zero knowledge** of what buttons do. It just detects hovers and clicks, and enqueues events. The event system handles the dispatch.

#### 4.1.7.5 EventQueue: I Could No Longer Pospone The Event Bus

Lucky for me, this was not a big deal. A simple queue decouples button interaction from game state:

```cpp
struct GameEvent {
	enum class Type { ButtonClicked, /* others */ };
	Type type;
	ButtonActionType buttonAction;  // Only populated for ButtonClicked
};

class EventQueue {
private:
	std::vector<GameEvent> _events;
public:
	void enqueue(const GameEvent& event);
	const std::vector<GameEvent>& getEvents() const;
	void clear();
};
```

This is functionally translated in each frame doing the following:
1. `UIInteractionSystem` enqueues events to the queue
2. `main` loop processes the queue
3. Queue is cleared

#### 4.1.7.6 MenuSystem: Query, Render, Rinse, Repeat

`MenuSystem` now focuses on spawning and rendering buttons. Nothing more, nothing less. Just the right amount of responsibility.

```cpp
class MenuSystem {
private:
	std::vector<Entity> _startButtonEntities;
	std::vector<Entity> _gameOverButtonEntities;
	
public:
	void setupStartButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table);
	void setupGameOverButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table);
	void clearStartButtons(Registry& registry);
	void clearGameOverButtons(Registry& registry);
	void buildStartMenuUI(Registry& registry, UIRenderQueue& queue);
	void buildGameOverUI(Registry& registry, UIRenderQueue& queue);
};
```

And because things are functionally simplified, the setup is also way more contained, with a rendering phase that just needs to query the `Registry`:

```cpp
void MenuSystem::setupStartButtons(Registry& registry, const ButtonConfigLoader::MenuButtonTable& table) {
	clearStartButtons(registry);
	_startButtonEntities = ButtonFactory::spawnButtonsFromConfig(registry, table, _screenWidth, _screenHeight);
}
```

```cpp
void MenuSystem::buildStartMenuUI(Registry& registry, UIRenderQueue& queue) {
	auto buttonView = registry.view<ButtonComponent, UIRenderableComponent>();
	
	for (Entity entity : buttonView) {
		// Only render if this button belongs to the start menu
		if (std::find(_startButtonEntities.begin(), _startButtonEntities.end(), entity) != _startButtonEntities.end()) {
			auto& button = registry.getComponent<ButtonComponent>(entity);
			
			// Enqueue render commands
			Color bg = button.hovered ? button.config.hoverColor : button.config.backgroundColor;
			Color outline = button.hovered ? button.config.outlineHoverColor : button.config.outlineColor;
			
			queue.rects.push_back(UIRectCmd{ button.bounds, bg, false, 0 });
			queue.rects.push_back(UIRectCmd{ button.bounds, outline, true, button.config.outlineThickness });
			queue.texts.push_back(UITextCmd{ button.config.label, button.bounds.x + button.bounds.width * 0.5f, ..., button.config.fontSize, ... });
		}
	}
}
```

The overall pattern is: **query the buttons, extract their data, enqueue render commands**. No state mutation. No side effects. Pure production of rendering intent. Pure refinement (*? I certainly hope so, this was hard to code tbh*).

#### 4.1.7.7 Main Loop Integration

State transitions now trigger button setup:

```cpp
// Track previous state to detect state changes
static GameState previousState = GameState::Menu;
if (state == GameState::Menu && previousState != GameState::Menu) {
	menuSystem.setupStartButtons(registry, menuButtons.start);
}
if (state == GameState::Playing && previousState != GameState::Playing) {
	GameManager::resetGame(registry, inputSystem, playerSnake, secondSnake, food, GRID_W, GRID_H, arena, AIPresets, mode);
}
if (state == GameState::GameOver && previousState != GameState::GameOver) {
	menuSystem.setupGameOverButtons(registry, menuButtons.gameOver);
}
previousState = state;
```

And he update phase is expanded so that it calls the interaction system:

```cpp
switch (state) {
	case GameState::Menu:
	case GameState::GameOver:
		uiInteractionSystem.update(registry, eventQueue);
		break;
	case GameState::Playing:
		inputSystem.update(registry);
		// ... other systems
		break;
}
```

And the event processing is straightforward:

```cpp
for (const auto& event : eventQueue.getEvents()) {
	if (event.type == GameEvent::Type::ButtonClicked) {
		switch (event.buttonAction) {
			case ButtonActionType::StartGame:
				state = GameState::Playing;
				break;
			case ButtonActionType::ChangeMode:
				mode = (mode == GameMode::SINGLE) ? GameMode::MULTI : GameMode::SINGLE;
				break;
			case ButtonActionType::Quit:
				state = GameState::Exiting;
				break;
			case ButtonActionType::ReturnToMenu:
				GameManager::resetGame(registry, ...);
				state = GameState::Menu;
				break;
		}
	}
}
eventQueue.clear();
```

And the UI rendering phase enqueues commands:

```cpp
case GameState::Menu:
	menuSystem.buildStartMenuUI(registry, uiQueue);
	uiSystem.renderRects(uiQueue);      // Draw rectangles
	textSystem.render(uiQueue);         // Draw text
	break;
case GameState::GameOver:
	menuSystem.buildGameOverUI(registry, uiQueue);
	uiSystem.renderRects(uiQueue);
	textSystem.render(uiQueue);
	break;
```

And I'm happy with all of this.

<br>
<br>

# 4.2 - The Current State of Main Things (or The Main State of Current Things)
After a lot of porting sheanenigans (how the hell is this word written?), everything worked pretty much correctly, but there were some syncronization issues along the different phases that comprise the `main` game loop. Transitions from menus into gameplay and back were funcionally fine, but I was getting some visual bugs related to a bad order management, things like frame flashing of uncomplete transitions. For example, the arena based wall rendering was producing a visual layout transition in the first frame when going from the start menu into gameplay, but not a fully update of its position (basically, a game world state complete check), which resulted in a visual, mid-transition-frame bug. In the way out, from gameplay into gameover, the bug was more subtle but still annoying (and, as everything, under the infinte gaze of God's judgement): the drawn walls changed colors (from `wallColor` to `customWhite`) first, then everything else. These issues, as I mentioned, pointed to the need of re-evaluating the game loop and its order of operations, which was rewritten as follows:
```cpp
while (true) {
		if (state == GameState::Exiting || WindowShouldClose()) break;

		const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

		if (IsKeyPressed(KEY_F)) ToggleFullscreen();
		if (IsKeyPressed(KEY_ONE)) renderMode = RenderMode::MODE2D;
		if (IsKeyPressed(KEY_TWO)) renderMode = RenderMode::MODE3D;
		if (IsKeyPressed(KEY_P)) postProcessingSystem.togglePostprocessing();
		if (IsKeyPressed(KEY_TAB)) {
			currentPresetIndex = (currentPresetIndex + 1) % static_cast<int>(arenaPresetList.size());
			animationSystem.notifyArenaSpawning(arena);
			arena.transformArenaWithPreset(arenaPresetList[currentPresetIndex]);
			float lineLifetime = 1.0f / animationSystem.getAnimationSpeed();
			arena.beginSpawn(lineLifetime);
		}

		// 1) Gather UI input for menu-like states (fills eventQueue)
		if (state == GameState::Menu || state == GameState::GameOver) {
			uiInteractionSystem.update(registry, eventQueue);
		}

		// 2) Process button events (state/mode mutations only)
		for (const auto& event : eventQueue.getEvents()) {
			switch (event.type) {
				case GameEvent::Type::ButtonClicked:
					switch (event.buttonAction) {
						case ButtonActionType::StartGame:
							state = GameState::Playing;
							break;
						case ButtonActionType::ChangeMode:
							switch (mode) {
								case GameMode::SINGLE: mode = GameMode::MULTI; break;
								case GameMode::MULTI:  mode = GameMode::VSAI;  break;
								case GameMode::VSAI:   mode = GameMode::SINGLE; break;
							}
							break;
						case ButtonActionType::Quit:
							state = GameState::Exiting;
							break;
						case ButtonActionType::ReturnToMenu:
							GameManager::resetGame(registry, inputSystem, playerSnake, secondSnake, food,
												GRID_W, GRID_H, arena, AIPresets, mode);
							state = GameState::Menu;
							break;
					}
					break;
				default:
					break;
			}
		}
		eventQueue.clear();

		// 3) Apply transitions BEFORE building context
		const bool transitionedThisFrame = (state != previousState);
		applyStateTransitionEffects(previousState, state, transitionContext);

		// 4) Build fresh context from current state/arena
		FrameContext ctx;
		ctx.arena      = &arena;
		ctx.state      = &state;
		const bool menuLikeState = (state == GameState::Menu || state == GameState::GameOver);
		ctx.menuLikeFrame = menuLikeState;
		ctx.gridWidth  = menuLikeState ? MENU_W : GRID_W;
		ctx.gridHeight = menuLikeState ? MENU_H : GRID_H;
		ctx.renderMode = &renderMode;
		ctx.playerDied = false;

		renderSystem.fillContext(ctx, &state);
		animationSystem.init(
			SCREEN_W, SCREEN_H,
			static_cast<int>(ctx.arenaBounds.x),
			static_cast<int>(ctx.arenaBounds.y),
			ctx.cellSize
		);

		// 5) UPDATE phase
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				break;

			case GameState::Playing:
				if (!transitionedThisFrame) {
					inputSystem.update(registry);
					aiSystem.update(registry, ctx);
					movementSystem.update(registry, dt);
					collisionSystem.update(registry, ruleTable, dispatcher, ctx);
					arena.tickSpawnTimer(dt);
					arena.tickDespawnTimer(dt);
				}
				particleSystem.update(dt, registry, ctx);
				animationSystem.update(dt, arena);
				break;

			case GameState::Paused:
				break;

			case GameState::Exiting:
				break;
		}

		// Death check happens after update phase
		if (ctx.playerDied) {
			std::cout << "PLAYER DIED" << std::endl;
			state = GameState::GameOver;
		}

		// 6) RENDER phase
		postProcessingSystem.beginCapture();
		switch (state) {
			case GameState::Menu:
			case GameState::GameOver:
				renderSystem.beginMode2D();
				animationSystem.render();
				particleSystem.render();
				renderSystem.renderMenu(ctx);
				renderSystem.endMode2D();
				break;

			case GameState::Playing:
				if (ctx.renderMode && *ctx.renderMode == RenderMode::MODE2D) {
					renderSystem.beginMode2D();
					animationSystem.render();
					particleSystem.render();
					renderSystem.render2D(registry, ctx);
					renderSystem.endMode2D();
				} else {
					renderSystem.render(registry, dt, ctx);
				}
				break;

			case GameState::Paused:
			case GameState::Exiting:
				break;
		}

		// 7) UI phase
		uiQueue.clear();
		switch (state) {
			case GameState::Menu:
				menuSystem.buildStartMenuUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				break;
			case GameState::GameOver:
				menuSystem.buildGameOverUI(registry, uiQueue);
				uiSystem.renderRects(uiQueue);
				textSystem.render(uiQueue);
				break;
			case GameState::Playing:
			case GameState::Paused:
			case GameState::Exiting:
				break;
		}

		postProcessingSystem.endCapture();

		// 8) PRESENTING phase (post processing et al)
		BeginDrawing();
		ClearBackground(customBlack);
		postProcessingSystem.applyAndPresent(dt);
		DrawFPS(SCREEN_W - 95, 10);
		EndDrawing();
	}
```
In broadstrokes, what needed to be restructured was the sequence of `ctx` building and management, the state related updates and click processing, the transition applies and the rendering branch calls. A more robust structure was needed to ensure that sequence coherence, so that this new order could be set up:
1. State change
2. Transition effects
3. Context rebuild
4. Update/render
5. Presentation

The key was to **skip gameplay update once if a state transition happens** to avoid mixed data (old/new). Which in itself meant taking care of these:
- Making sure that **no frame can render with `state=Playing` but `ctx/menu layout` from a previous state
- Assuring that **transition side effects stay atomic for their initial frame**
- Rely on the optional one-frame update skip to **prevent stale entities/particles from stepping in a mixed transition frame**.

For the `Playing -> GameOver` case in particular, there is one additional little guard worth noting: the wall color decision is no longer pulled from a potentially already-mutated live `state` pointer in the render pass. Instead, the frame carries a snapshot bool (`ctx.menuLikeFrame`) built together with the rest of the context, so the visual mode remains consistent for that whole frame. This is what prevented the subtle one-frame white-wall flash while keeping the transition order intact.

In practice, the new `main` loop is shaped like this:
1. Poll menu input (`uiInteractionSystem`) first (but just for menu/gameover)
2. Process events and mutate the `state` if necessary
3. Apply transition effects
4. Build `ctx` **AFTER** any possible state transition
5. Run simulation updates with a fresh `ctx`
6. Render everything
7. Present the render (post process and display it)

> *This is a good spot to end this long ass log. There's a couple things left in the port, but I'll take them to the next one (5). See you there.*