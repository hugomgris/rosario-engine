# Rosario - Devlog - 4

## Table of Contents
1. [Back to the Porture](#41---back-to-the-porture)

<br>
<br>


# 4.1 - Back to the Porture
As stated at the end of the third log, it's time to go back to porting systems, as well as some other stuff that is still pending in the already ported one (like the 3D rendering pipeline, for example). Let's start with recovering the `AI`, which is going to need for a new `AIComponent`, some helpers and, of course, an `AISystem`. And because we're now all json-y, we'll take what were code-stated AI configurations into an `AIPresets.json` data file, which will need an `AIPresetLoader`. Nothing new, same old.

### Raise Again, Green Snake
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

### Yellow Snake, Please Go to Aisle Game
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