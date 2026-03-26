# Rosario - Devlog - 6

## Table of Contents
1. [Back to Testlandia](#61---back-to-testlandia)
2. [Testiary](#62-testiary)
    - [Unit Tests](#62-unit-tests)
        - [Core ECS](#621-unit-tests---core-ecs)
        - [Data Loaders](#622-unit-tests---data-loaders)
        - [AI Pathfinding](#623-unit-tests---ai-pathfinding--grid)
        - [Arena](#624-unit-tests---unit-tests---arena)
        - [Collision System](#625-unit-tests---collision-system)
        - [Components](#626-unit-tests---components)
        - [Configuration Structs](#627-unit-tests---configuration-structs)
    - [Implementation Tests](#63-integration-tests)
        - [ECS System](#631-integration-tests---ecs-system)


<br>
<br>

# 6.1 - Back to Testlandia
I had no plans to log anything related to the test building. That was until one of the first planned tests (from a list of roughly 130) revealed a fundamental error (well, more like a missing step) in the core of the ECS structure, related to entity management in the `Registry`. Therefore, as I predict that this might happen several more times down the line (or not, maybe I'm that good and I only made ONE unit of mistake, huh???), I'm going to spend some time writing about (auto)testing once again.

<br>

## Once More Around the Gtest Sun (@mastodon)
Back when I was coding [nibbler](https://github.com/hugomgris/nibbler), I thoroughly wrote about how to build an automated test suite (specifically, a gtest based c++ test suite with unit and integration tests). If you're reading this and you have never written your own test suite, and you have never done, it, my first recommendation would be to go back to the [13th devlog](https://github.com/hugomgris/nibbler/blob/main/docs/devlog_13.md#132-testing-testng-one-two-three) to get an introduction (or any other source of your choice, that is). In this document I'll just refresh some specifics, like naming convention, and log any errors, bugs, mistakes, miseries that the 130+ planned tests reveal.

To me, at this point in life, the most difficult step when building a test suite is the foundational question: **what the hell should I test?** This is more difficult than it could seem, as the code that you are attempting to test might be something you fully know and have control of, but imagination has to be put in service of **how your code might fail**, all while writing tests based on predictable outcomes. On some level this *really* isn't that complicated. For example, I'm going to start my suit by testing the core, writing simple unit tests that target one, and just *ONE* implementation around my `Registry`. Some of the first tests couldn't be simpler: let's see if the pipeline to create entities, register them and juggle components attached to them works, let's see if destroying an entity results in a full cleanup of its existence in the registry, let's see if that `forEach` function I added at some point really works and runs the sent function, etc. Those who aim to be very thorough, as should be the case for any test suite because if not what's the point, should go through the structure of the program, it's components and layers, check their contents and functions and basically stress every little component there is. If my `Registry` has a query function to check if a given entity has this or that component attached, it should be tested. If it has a boolean check for the existance of a specific component pool, tested it must be. Every getter, every checker, every remover, every constructor, every whatever *needs* to be tested. So spending some time making a to-do list of things to test by just combing your code is a good way to start. A miserable, time consuming one, but effective.

This is easier, obviously, when planning unit tests. Integration tests are a whole deal on themselves, as you need to think on higher layers of possible interaction inside your program to figure out tests cases. I feel that the ECS approach kinda helps in this regards, as having a clear structure of systems gives more transparency (or immediacy, if you prefer) to said interactions. Is your X system working in cooperation with your Y system? Should your Z system block your X system in any way? What is the order of update calls in XYZ? All of these questions call for integration test planning. Basically, you have to ask yourself: **what functional couplings can I identify in my flows?**. This doesn't mean that just by looking at the code you should come up with a load of tests. This is a continuous process: as you write tests, new cases will come to mind.

There's another way of doing this, one that I'm particularly fond of but that is only really advisable (to me, that is, as everything I write) in cases that really benefit for it. I'm talking about **Test Driven Development**, a specific way of building software that, with functionality laid out, thinks about implementation tests that initially fail (obviously, they exist before the functionality they target) and then codes with passing those tests as objectives. I did this back in my [C# Computorv2](https://github.com/hugomgris/computorv2), mainly because what that program needed to do was perfectly clear and delimited: I knew that if I sent "2+2=?" as argument to the program, it should return 4, so I could write an assertion base test in that direction. Then, I just wrote the logic so that the test passed. Then I amped the stress: what about "2+-2=?". Back to test failing, then passing-bound code writing. And so on. If this is development approach that suits you and your project, it is a great way to kind of kill 2 birds with one stone, as any progress comes with an already laid out test suite section. All that while remembering that this is not always the best course of action: you need to asses, decide, then plan.

There's obviously an in-between possibility. A well planned development with clear progress steps lets you code part of your program first, test it then, rinse and repeat. I guess this is the most rational way of doing things, and I would love to have done that. But I didn't, so now I have to spend a couple of days in Testlandia crying my way through my cursed test-to-do list. It is what it is, for me. You, I hope, can still be saved.

## Naming Names
With the reality of programming being a cooperative process in the rearview mirror (it depends, I know; you might be coding in solitude, but you will most likely have to integrate yourself in a plural work structure), it is very important to think about how to name your tests. What looks like an extended convention in this regard, and keeping ourselves bounded to `GoogleTest` (in the sense that this convention might not apply to your language+suite combination, give it a research), is:
- Suite name = feature or unit under test
- Test name = specific behavior or rule being verified
- Use specification = clear, comprehensive explanation of what is expected (PascalCase)
- Think about sentences more than words. It might look weird, but I can't emphasize enough that a test suit that is not clear in its intents, targets and scopes is useless.

With all this in mind, an example would look like this:
```cpp
TEST(SubjectOrFeature, ShouldExpectedBehavior_WhenCondition) {}
```

Which in my specific suite traslates into the following:
```cpp
TEST(Registry, ShouldAddGetAndRemoveComponent_WhenSingleComponentLifecycle) {}
TEST(Registry, ShouldMutateOnlyMatchingComponentType_WhenUsingForEachSingleType) {}
TEST(Registry, ShouldSupportCopyAndMoveInsertion_WhenAddingComponents){}
```

It goes without saying that you do you, this is just my humble attempt to lay out what I think are good practices regarding all this test writing stuff. And with all this said and behind us, I can focus on test writing itself and logging found errors.

<br>
<br>

# 6.2 Testiary
*As in Bestiary, heh*. I'll go section by section through my test-to-do list and register any errors they rise. This might be a long log, so buckle up. And if it ends up not being that long, good for me, that means that my code was awesome and prime and fantastic and job-worthy and you should call me so that we work together.

## 6.2 Unit Tests
### 6.2.1 Unit Tests - Core ECS
The first batch of tests will focus on the `Registry` management, stressing the ECS core, and juggling entities and components around.

- [x] Registry: Multiple component types on single entity
- [x] Registry: Component pool isolation (one component type doesn't affect another)
- [x] Registry: Entity destruction and cleanup (no dangling references)
- [x] Registry: hasComponent() returns false after removeComponent()
- [x] Registry: forEach() with single component type
- [x] Registry: view() with multiple component filters
- [x] Registry: Attempting to get non-existent component throws exception
- [x] Registry: Accessing destroyed entity throws exception
- [x] ComponentPool: Capacity growth under stress (many entities)
- [x] ComponentPool: Copy and move semantics for components

#### Issue found:
- Entity destruction was not thorough. `Registry`'s `destroyEntity()` function was clearing the target entity from the registry's entity list, but not from the component pools. A simple fix to what was a very important bug:
```cpp
void Registry::destroyEntity(Entity entity) {
	auto it = std::remove(_entities.begin(), _entities.end(), entity);
	_entities.erase(it, _entities.end());

	for (auto& [_, pool] : _componentPools) {
		pool->removeEntity(entity);
	}
}
```

### 6.2.2 Unit Tests - Data Loaders
The second batch of tests will target all the data loaders set up for the `JSON` → `struct` pipeline. A lot of parsing, excepcion and edge case stress and missing/corrupted data reactions. This batch will be split in multiple test files, one for each loader.

- [x] ParticleConfigLoader: Direction enum parsing (UP/DOWN/LEFT/RIGHT)
- [x] ParticleConfigLoader: Malformed JSON throws exception
- [x] ParticleConfigLoader: Missing required fields throw exception
- [x] ParticleConfigLoader: Incomplete values throw exception
- [x] ParticleConfigLoader: Out-of-range values throw exception
- [x] AIPresetLoader: Parse AI presets with difficulty levels
- [x] AIPresetLoader: AIPreset data structures match config
- [x] AIPresetLoader: Invalid difficulty level throws exception
- [x] ArenaPresetLoader: Parse arena preset configurations
- [x] ArenaPresetLoader: Wall matrix dimensions valid
- [x] CollisionRuleLoader: Parse collision rules from JSON
- [x] CollisionRuleLoader: Subject/object pairs correctly stored
- [x] CollisionRuleLoader: Malformed rules throw exception
- [x] TunnelConfigLoader: Parse tunnel animation config
- [x] PostProcessingConfigLoader: Parse post-processing effect settings
- [x] All Loaders: Missing config file throws appropriate exception
- [x] All Loaders: Corrupted JSON throws parsing exception

#### Issues found:
- The test regarding the wall matrix dimension validity signaled that something was off in the way I was loading arena presets. While trying to write the test I realized that it wasn´t *making sense* because the definition of the presets was in the side of the code, not in the json file. This instantly made me think that the way I was managing preset coniguration was not in line with all my other loading pipelines. So a refactoring was due.
	- I took the opportunity to redefine how the presets were set up, now in the `JSON` files. Instead of defining values via accessing and what not, I now set up the presets with specific characters in the config files. Here's an example comparison:
> BEFORE
```cpp
void ArenaPresets::applySpiral1(ArenaGrid& arena) {
    const int gap = 3;

    int W = arena.getPlayWidth();
    int H = arena.getPlayHeight();

    int limTop    = -1;
    int limLeft   = -1;
    int limBottom = H;
    int limRight  = W;

    int x = W - 1 - gap;
    int y = gap;

    auto hline = [&](int x0, int x1, int yy) {
        if (x0 > x1) std::swap(x0, x1);
        for (int xx = x0; xx <= x1; ++xx)
            arena.setCell(xx, yy, CellType::SpawningSolid);
    };
    auto vline = [&](int xx, int y0, int y1) {
        if (y0 > y1) std::swap(y0, y1);
        for (int yy = y0; yy <= y1; ++yy)
            arena.setCell(xx, yy, CellType::SpawningSolid);
    };

    enum Dir { LEFT, DOWN, RIGHT, UP };
    Dir dir = LEFT;

    for (int iter = 0; iter < 400; ++iter) {
        switch (dir) {
            case LEFT: {
                int target = limLeft + 1 + gap;
                if (target > x) return;
                hline(target, x, y);
                limTop = y;
                x = target;
                dir = DOWN;
                break;
            }
            case DOWN: {
                int target = limBottom - 1 - gap;
                if (target < y) return;
                vline(x, y, target);
                limLeft = x;
                y = target;
                dir = RIGHT;
                break;
            }
            case RIGHT: {
                int target = limRight - 1 - gap;
                if (target < x) return;
                hline(x, target, y);
                limBottom = y;
                x = target;
                dir = UP;
                break;
            }
            case UP: {
                int target = limTop + 1 + gap;
                if (target > y) return;
                vline(x, target, y);
                limRight = x;
                y = target;
                dir = LEFT;
                break;
            }
        }
    }
}
```

> AFTER

```json
{
	"name": "Spiral1",
	"width": 32,
	"height": 32,
	"walls": [
		"................................",
		"................................",
		"................................",
		"...##########################...",
		"...#............................",
		"...#............................",
		"...#............................",
		"...#...######################...",
		"...#...#....................#...",
		"...#...#....................#...",
		"...#...#....................#...",
		"...#...#...##############...#...",
		"...#...#...#............#...#...",
		"...#...#...#............#...#...",
		"...#...#...#............#...#...",
		"...#...#...#...######...#...#...",
		"...#...#...#...##...#...#...#...",
		"...#...#...#........#...#...#...",
		"...#...#...#........#...#...#...",
		"...#...#...#........#...#...#...",
		"...#...#...##########...#...#...",
		"...#...#................#...#...",
		"...#...#................#...#...",
		"...#...#................#...#...",
		"...#...##################...#...",
		"...#........................#...",
		"...#........................#...",
		"...#........................#...",
		"...##########################...",
		"................................",
		"................................",
		"................................"
	]
}
```
The comparison speaks for it self, I must say. And the beforementioned test case that triggered this transformation now makes completely sense, I must add. And the build passes it with flying colors, I must end this silly sentences with.

Ah, by the way, the rest of the changes in code that carry out this new way of loading arena presets are just your already classic loading, storing, reading, applying actions, all chained together in the same way as every other loading sub-pipeline in this new version of the engine/game.

### 6.2.3 Unit Tests - AI Pathfinding & Grid
The third batch of tests will stress test the AI pipelines to find its way through the game arena. To that aim, three main targets need to be put under the spotlight: `GridHelper`, `FloodFill` and `PathFinder`. Let's do this!

- [x] GridHelper: Grid coordinate validation (bounds checking)
- [x] GridHelper: Neighbor generation for interior cell
- [x] GridHelper: Neighbor generation at grid edges
- [x] GridHelper: Neighbor generation at grid corners
- [x] GridHelper: Manhattan distance calculation
- [x] FloodFill: Count reachable cells on empty grid
- [x] FloodFill: Count reachable cells with obstacles
- [x] FloodFill: Count reachable from various start positions
- [x] FloodFill: Properly ignores specified positions
- [x] FloodFill: Detect unreachable areas (isolated regions)
- [x] FloodFill: Single cell reachable when surrounded (but not blocked itself)
- [x] FloodFill: canReachTail() returns true when tail reachable
- [x] FloodFill: canReachTail() returns false when tail blocked
- [x] Pathfinder: Find path on empty grid (straight line)
- [x] Pathfinder: Find path with single obstacle (circumnavigate)
- [x] Pathfinder: Find path with complex obstacle maze
- [x] Pathfinder: No path returns empty vector
- [x] Pathfinder: Respects maxDepth limit
- [x] Pathfinder: Path is shortest (or near-optimal with A*)
- [x] Pathfinder: Properly ignores specified positions
- [x] Pathfinder: Start equals goal returns path of length 0
- [x] Pathfinder: No backtracking in returned path

#### Issues found:
- While writing a test that returned/forced a `false` return in my `canReachTail()` function, the only way I could come across was to completely enclose the `aiSnake` in obstacles. This lead me to the good old feeling of *uhm, this is fishy*. Looking into the function, the way that I was calculating the reachable tail status was by amount comparisson between the needed space and available space, which is... not ideal, lol. This is what I was doing:
```cpp
bool FloodFill::canReachTail(const Registry& registry,
                              const std::vector<std::vector<bool>>& blocked,
                              Entity aiEntity,
                              const std::vector<Vec2>& proposedPath,
                              int gridWidth,
                              int gridHeight) const {
    if (proposedPath.empty())
        return false;

    const auto& snake = registry.getComponent<SnakeComponent>(aiEntity);
    if (snake.segments.empty())
        return false;

    const Vec2 newHead     = proposedPath.back();
    const Vec2 currentTail = snake.segments.back().position;

    int reachable     = countReachable(blocked, newHead, gridWidth, gridHeight, { currentTail });
    const int required = static_cast<int>(snake.segments.size()) + 1;
    return reachable >= required;
}
```

This needed a full refactor, and a plunge into higher depths of complexity.

The conceptual issue was simple to phrase: I was answering a geometric/topological question (*"can head still reach tail after this move?"*) with a volumetric proxy (*"is there enough free space around head?"*). Useful, sure, even practical, but not sufficient. So a new strategy was due, which I wanted to base on a two-gate validation:
1. **Truth gate**: simulate the future snake state and explicitly check head->tail connectivity.
2. **Quality gate**: if connectivity exists, verify there is enough maneuver room (the old amount check, but now in the simulated state, not the current one).

In other words: no more "big area therefore safe" shortcutting. It was fun while it lasted. The resulting flow in `canReachTail()` now looks like this:

```cpp
bool FloodFill::canReachTail(/* ... */) const {
    if (proposedPath.empty())
        return false;

    const auto& snake = registry.getComponent<SnakeComponent>(aiEntity);
    if (snake.segments.empty())
        return false;

    // 1) simulate future body over proposedPath
    std::deque<Segment> futureSegments = snake.segments;
    bool growsThisStep = snake.growing;
    for (const Vec2& nextHead : proposedPath) {
        if (!isAdjacentCardinal(futureSegments.front().position, nextHead))
            return false;

        futureSegments.push_front({ nextHead, BeadType::None });
        if (growsThisStep) growsThisStep = false;
        else futureSegments.pop_back();
    }

    // 2) rebuild blocked map for that simulated snake
    // 3) Breadth-First Search (BFS) from future head, treating future tail as reachable target
    // 4) if tail is found, run secondary reachable-space threshold
}
```

The key thing here is **simulation**. Instead of looking at what exists *now*, we construct what would exist *if that path is committed*. That includes body shift and growth behavior (`snake.growing`) matching the movement system's logic.

Then comes another important detail: when rebuilding the temporary blocked grid, the function clears current snake occupancy and writes the simulated occupancy. This avoids stale body coordinates polluting the check.

And then, the central check itself is now explicit Breadth-First Search (BFS) connectivity:
- start from simulated head
- explore cardinal neighbors
- use `isWalkable(..., {futureTail})` so the tail can be stepped into as the target
- if BFS never reaches tail, return `false`

Only after the tail is proven reachable do we run the second gate:

```cpp
const int reachable = countReachable(
    futureBlocked,
    futureHead,
    gridWidth,
    gridHeight,
    { futureTail }
);
const int required = static_cast<int>(futureSegments.size()) + 1;
return reachable >= required;
```

So, in reality, the old amount comparison wasn't deleted, but demoted from "truth criterion" to "quality safeguard". Which, frankly, is exactly where it should have been from day one, but day one stuff is never done on day one, right? That's just a backwards evaluation, right???

Anyway, one (GOOD) side effect is that invalid proposed paths are now rejected early. If a path includes non-adjacent steps, the function returns `false` immediately. That gave me stricter behavior and cleaner intent.And because tests are merciless and beautiful, this also forced me to revisit the blocked-tail test expectation and remove debug leftovers. After alignment, the whole suite passed again, and the final behavior now matches the real question we wanted to ask in the first place: *can the snake reach its own tail?*

> *Another side effect is that the game now feels more difficult, or at least that the AI snake seems to be smarter. Maybe it's not true and it's just a biased feeling, who knows*

### 6.2.4 Unit Tests - Unit Tests - Arena
The fourth batch of tests aims to put the `ArenaGrid` under the microscope. Its setters and getters, cntent checks, preset based transformations and management functions will be thoroughly examined.

- [x] ArenaGrid: Constructor initializes grid with correct dimensions
- [x] ArenaGrid: setCell/getCell basic get/set operations
- [x] ArenaGrid: getCell bounds check (returns error/exception for out-of-bounds)
- [x] ArenaGrid: isWalkable() returns correct value for each CellType
- [x] ArenaGrid: getAvailableCells() returns only Empty cells
- [x] ArenaGrid: getAvailableCells() count matches expected empty cell count
- [x] ArenaGrid: clearCell() sets cell to Empty
- [x] ArenaGrid: clearArena() empties all cells
- [x] ArenaGrid: spawnObstacle() creates rectangular obstacle
- [x] ArenaGrid: growWall() adds wall to grid
- [x] ArenaGrid: transformArenaWithPreset(InterLock1) layout is correct
- [x] ArenaGrid: transformArenaWithPreset(Spiral1) layout is correct
- [x] ArenaGrid: transformArenaWithPreset(Columns1) layout is correct
- [x] ArenaGrid: transformArenaWithPreset(Maze) layout is correct
- [x] ArenaGrid: getAllOutlines() returns correct outline vectors
- [x] ArenaGrid: beginSpawn() initiates spawn timer
- [x] ArenaGrid: setMenuArena() vs setGameplayArena() produce different layouts
- [x] ArenaGrid: Wall preset boundaries (perimeter stays intact)

NO issues found!

### 6.2.5 Unit Tests - Collision System
The fifth batch will be shorter, a handful of handling checks to review rule management, dispatch correctness and different effect presets (multi effects, invalid effect).

- [x] CollisionRule struct construction and field assignment
- [x] CollisionRuleTable::find() returns correct rule for subject/object pair
- [x] CollisionRuleTable::find() returns nullptr for unknown pair
- [x] CollisionEffectDispatcher::dispatchEffect() applies correct effects
- [x] CollisionEffectDispatcher: Multiple effects on single collision
- [x] CollisionEffectDispatcher: Invalid effect name throws/logs error

Once more, NO issues found!

### 6.2.6 Unit Tests - Components
The sixth batch is all about components, specifically those which make sense to put through the suite churn machine: `SnakeComponent`, `PositionComponent`, `ScoreComponent`, `MovementComponent`, `InputComponent`, `RenderComponent`. 

- [x] SnakeComponent: Add segments to snake
- [x] SnakeComponent: Remove segments from snake
- [x] SnakeComponent: Segment count matches constructor/modifications
- [x] SnakeComponent: Powerup bead assignment (Shield, Speed, Ghost, Ram)
- [x] SnakeComponent: BeadType enum values valid
- [x] PositionComponent: Grid coordinates within bounds
- [x] PositionComponent: Coordinate getters return correct values
- [x] ScoreComponent: Increment score correctly
- [x] MovementComponent: Direction enum assignment (UP/DOWN/LEFT/RIGHT)
- [x] InputComponent: Input queue operations
- [x] RenderComponent: Tint color assignment

Happily enough, NO issues here!

### 6.2.7 Unit Tests - Configuration Structs
The seventh and final unit test batch could feel redundant, but I wanted to closely look into the construction of configuration structs. We'll go back to that point in which the json based data gets transfromed into code-sided struct data.

- [x] ParticleConfig: All sub-configs accessible
- [x] ParticleConfig: MenuTrailPreset vector operations
- [x] MenuTrailPreset: Color and direction assignment
- [x] MenuTrailPreset: Manual position (x, y) handled correctly
- [x] ParticleConfig: Default values applied correctly

NO issues and unit tests done!!

<br>

## 6.3 Integration Tests
### 6.3.1 Integration Tests - Config + Runtime Wiring
We'll start the integration tests by going to the first execution steps and checking out how the config loading and its wiring with different gameplay system answers to different cases. The general idea when attempting to write integration tests is to think about *chunks* of the whole program that can be isolated to make them work independently. This is going to be the way I'm going to think about these, at least, all across the 6.3 subsections, which in this particular first one translates into taking out the initial preparation steps of the game execution and asserting that all the critical points are correctly managed.

- [x] Boot loads all runtime JSON files together and binds them to systems (no cross-loader interference)
- [x] Hot reload glyph presets updates visible UI text entities without restarting
- [x] Hot reload fallback path works when a preset is removed (template fallback still renders)
- [x] Reloading particle config updates menu trail behavior on next frame (direction/color/interval)

NO issues!

### 6.3.2 Integration tests - UI + Pixel Text + Menu Particles
Writing integration tests is proving to be difficult. The process of isolating, first, and targetting very specific interactions between program chunks, then, is menacing my mental sanity, and even though the sub-sections in this 6.3 are way shorter than in the unit test one, it's taking quite a lot of time. The trade off is that writing this tests requires a deep thought process following the execution flow, so there's a progressive in depth control of the code building suite after suite. At least that's how I'm feeling it, as it seems that having a general mind scheme about how a program works, i.e. the sum of its parts, and about those parts themselves is not self-guaranteed. What I mean by this is that reproducing micro-behaviours and interrogating them is a battle. And I'm fighting it. AND WINNING IT.

- [x] Menu keyboard/mouse co-navigation preserves single hover source of truth across frames
- [x] State-scoped pixel text visibility toggles correctly between Menu and GameOver
- [x] Menu logo trail requests follow text visibility and stop when logo is hidden
- [x] Multiple menuTrail emitters run independently with per-emitter 

ZERO issues. Heck yeah.

### 6.3.3 Integration tests - Rendering and Frame Pipeline
These tests will check the system update sequence at each frame generation in the game loop. Because the game has different states, what will follow will be different versions of a same test in each of those states, closed by a frame consistency check.

- [x] One full frame in Menu state runs update+render with no crashes
- [x] One full frame in Playing state runs update+render with no crashes
- [x] One full frame in GameOver state runs update+render with no crashes
- [x] Multiple consecutive frames maintain state consistency

NO issues. No celebration, either, as it is starting to be the norm. YEAAAAAAAAAH

### 6.3.4 Integration tests - State Transitions and World Rehydration
Now is the turn for the `StateMachine` and `Registry` content juggling when navigating through game states. How the world resets its content, how elements that need clearing are flushed during transitions, how elements that need to be kept are saved from wipes, and a close look at the possibility of unwanted duplication of entities.

- [x] Menu -> Playing transition resets world and spawns expected gameplay entities
- [x] Playing -> GameOver transition swaps UI layout and state-visible text correctly
- [x] ReturnToMenu action clears gameplay entities and restores menu entities cleanly
- [x] Registry reset during transition does not leave stale references in UI/pixel text helpers
- [x] Re-entering Playing multiple times does not duplicate persistent entities

Issues = 0

### 6.3.5 Integration tests - Input -> Decision -> Movement Chain
These are somewhat easier tests to deploy. Shorter, more straight forward, centered around input buffering and consuming. The main idea here is that the buffering works, the user-game interaction should do too.

- [x] Player input buffering is consumed across frames in deterministic order
- [x] Opposite-direction rejection remains correct across multiple buffered inputs
- [x] VS-AI mode: AI decision output is reflected in movement on the next tick

Total Abscence of Issues

### 6.3.6 Integration tests - Collision -> Effects -> Gameplay Outcome
These are sort of straight forward too, but require a handful of parts to be setup, as it is an array of wide interaction cases. Nothing too complicated, thankfully, as the collision logic setup was already checked in its respective unit test and what really needs to be tested now is the handling, dispatching and resulting effects of their management.

- [x] Snake-Food collision applies full effect chain (grow + score + relocate food)
- [x] Snake-Wall collision transitions gameplay to death/gameover path
- [x] Snake-Self collision transitions gameplay to death/gameover path
- [x] Multi-effect collision dispatch is idempotent per tick (no double application)

Everything seems to be issueless

### 6.3.7 Integration tests - Arena + AI Coherence
This batch focuses on the handshake between spatial data and AI decision systems. The objective is to verify that arena mutations are reflected immediately in the AI blocked-grid view, and that pathfinding/safety checks stay coherent when the board changes during runtime. In practical terms, these tests ask one core question: does the AI reason about the same world the gameplay systems are currently rendering and updating?

- [x] Arena preset application is reflected in AI blocked-grid view within same update cycle
- [x] Path selected by AI remains walkable after movement step validation
- [x] Flood-fill safety check and pathfinder decision stay consistent on identical board state
- [x] AI repaths after dynamic arena changes (spawn/despawn solids) without freezing

ISSUEFREE

### 6.3.8 Integration tests - Rendering and Frame Pipeline
This batch verifies rendering-state coherence when visual configuration changes at runtime. The focus is not on pixel-perfect output, but on update/render contract stability: mode switches should not break system execution order, post-processing toggles should not corrupt UI layering assumptions, and tunnel/particle animations should coexist without leaking or cross-contaminating internal state.

- [x] Mode2D/Mode3D switch during gameplay preserves render/update coherence
- [x] Post-processing on/off toggle does not break UI composition order
- [x] Tunnel animation and particle rendering coexist without state leakage

I don't know how many ways of saying that there were no issues I have left in me

### 6.3.9 Integration tests - End-to-End Soak and Stability


- [x] 300-frame deterministic soak in Playing (fixed seed, scripted inputs) produces stable outcomes
- [x] Repeated state cycling (Menu <-> Playing <-> GameOver) for N loops has no leaks under Valgrind
- [x] Long-run entity churn (spawn/despawn gameplay objects across rounds) shows no invalid accesses
- [x] Config reload + gameplay transition sequence remains stable (no crash, no stale pointers)