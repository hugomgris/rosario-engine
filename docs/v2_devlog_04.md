# Rosario - Devlog - 4

## Table of Contents
1. [Back to the Porture](#41---back-to-the-porture)
	- [Raise Again, Green Snake](#411-raise-again-green-snake)
	- [Yellow Snake, Please Go to Aisle Game](#412-yellow-snake-please-go-to-aisle-game)
	- [Re-Diced and Re-Sliced](#413-re-diced-and-re-sliced)
	- [Post-Post-Processing](#414-post-post-processings)
	- [Why the Long Face?](#415-why-the-long-face)

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

