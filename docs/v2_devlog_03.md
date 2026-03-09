# Rosario - Devlog - 3

## Table of Contents
1. [Data Takes the Driver Seat](#31---data-takes-the-driver-seat)
2. [Data Driven Programming 101](#32---data-driven-programming-101)
3. [Transition = Plan](#33-transition--plan)
4. [Carefully Composing Components](#34-carefully-composing-components)
5. [Writing the Systems](#35-writing-the-systems)

<br>
<br>


# 3.1 - Data Takes the Driver Seat
Today we're going to make a huge roll back and rebuild everything from scratch. Why, you may ask. Because I want to learn about a paradigm that I haven't implemented before, and this is the best project and context to do so, I shall answer. The new, temporary objective: take our rosario into the realm of **Data Driven Programming**. So the plan is going to first go through a theoretical learning step, what this approach is, means and entrails, how to wrap the mind around it, where to start thinking about the new build. Then, you guessed it, coding, coding, coding. So let's sit down and start making notes!

<br>
<br>

# 3.2 - Data Driven Programming 101
The core of this paradigm is simple to state: **The code describes *how* things work, and data describes *what* things are**. In a traditional **OOP** approach, like what I've been building in this project, a `Snake` object ***knows*** it's a sanke: it has methods, state, and behaviour all bundled together. The ***identity* ad the *behaviour* are fused**. Flipping this approach, in data-driven programming we would have:
- **Entities**: just an Id. A number. Nothing else.
- **Components**: pure data structs. No methods. No logic.
- **Systems**: pure logic. No state. Operting on components.

Therefore, transitioning to a data-driven model would mean that **a "snake" stops being a class, and it becomes an entity that *happens to have*** a `PositionComponent`, a `MovementComponen`, a `SnakeBodyComponent`, etc. The entity itself has no idea it's a snake.

This matters (in itself, but also when considering it against an OOP approach), because in OOP you might have a `Snake` that has position, body, direction, AI logic, render logic, collision logic... And everything seems fine until you want:
- A snake that *sometimes* has AI and *sometimes* doesn't
- Food that *temporarily* behavaves like a snake segment
- Wall obstacles that share collision logic with snake bodies
- new modes where snake behaves differently than in default conditions (for example, a phasing snake, one that doesn't die at the edges of the arena but teleports, ...)

Basically, OOP is not the most manageable model when complexity and possibility scale, nor when several people need to work at the same, complex codebase without tripping on each others implementations (i.e., data-driven brings more inependence and flexibility to the work flows).

### ECS Mental Model
The paradigm, as stated before, is sustained by an **`E`ntity `C`omponent `S`ystem**, which as the name advances, is comprised of **three elements**.

#### Entity
```cpp
using Entity = uint32_t; // Just a number, nothing more
```

Entity `0` might be `Snake A`. Entity `1` might be a `Food` pip. Entity `2` might be a `Wall brick`. Whatever the case, the important, basic thing about entities is that **they don't know what they are**. They don't *need* to know.

#### Component
```cpp
// Pure DATA. No methods. No logic.
struct PositionComponent {
	int x, y;
};

struct MovementComponent {
	int dirX, dirY;
	float moveTimer;
	float moveInterval;
};

struct SnakeBodyComponent {
	std::vector<Vec2> segments;
	bool growing;
};
```
Each component contains data related to capacities and/or behaviors, but they don't contain the logic of whatever implementation their related to. A `PositionComponent` just contains the `X` and `Y` value for the positioning of an entity, but the act of positioning itself is delegated to the corresponding system.

#### System
```cpp
// Pure logic. Operates on whoever HAS the right components
class MovementSystem {
	public:
		void update(Registry& registry, float deltaTime) {
			for (auto entity : registry.view<PositionComponent, MovementComponent>()) {
				auto& pos = registry.get<PositionComponent>(entity);
				auto& move = registry.get<MovementComponent>(entity);
				// move logic
			}
		}
};
```

A system doesn't know or care if the entity is a snake, a food, a unicorn. If an entity has the necessary components, it gets affected by the system. Nothing more.

### Why, tho

In the context that I'm interested and focused on, game development, the main reason behind a data-driven approach development is plain and simple: complexity. Think about a more or less big game:
- Tens or hundreds of characters, items, spells, surfaces
- Any character can have any combination of statuses, abilities, equipment
- Multiplayer with several players, all affecting the same world simultaneously
- Modding support
- Whatever you can come up with

OOP would be a **n i g h t m a r e** at that scale. Imagine a class hierarchy trying to represent "a character who is on fire, posined, turned into a chicken, holding a legendary sword with a lot of stats, and currently being mind-controlled by an enemy AI". Cry in OOP tears. BUT, with ECS, one just has to add:
- `OnFireComponent`
- `PoisonedComponent`
- `ChickenFormComponent`
- `EquipmentComponent` with the sword referene
- `MindControlledComponent`

So each system that cares about fire, or posion, or whatever/etc, just checks for its relevant component. So clean, so composable, so absence of need for a lass hierarchy change. Trying to think about `rosario` (the *snake* game, really) in this terms would mean thinking about things like:
- An empty cell that *temporarily* becomes an obstacle (just swap the component!)
- A snake that enters "ghost mode" and has a phasing-through-obstacle transitional behavior
- The AI turned off for player-controlled snakes (it's just a snake without an AI component!)

### Cache Friendship Club
This might be a more advanced reason behind an ECS choice, one that could use some extended explanations, but for the sake of briefness and just to *know* about it: **in OOP objects are scattered in memory**. When you iterate over 100 snakes, the CPU has to jump around in RAM fetching each object, which means a lot of **cache misses**, which makes things s l  o   w. On the contrary, in ECS, all `PositionComponent`s are stored in a **packed array**, one after another. When the movement system iterates over them, the CPU loads them in a perfect stream, which means **cache friendlyness**, which makes things FAST. And this is why game engines like Unreal, Unity, Godot, and etc. move towards ECS for performance-critical systems.

<br>
<br>

## 3.3 Transition = Plan
With some basics in our backpack, it's time to plan how to make the transition from OOP to DD. Let's make it in stages, as traying to embrace everything everywhere all at once would be spirit-breaking and mind-blasting:
1. **Build the ECS Core**: Write `Entity`, `ComponentPool<T>`, and `Registry` from scratch. Nothing game-specific yet. This is the foundation.
2. **Port Components**: Take the existing data (position, direction, body segments, etc) and pull them out of the classes into pure structs.
3. **Port Systems**: Take the existing logic (movement, collision, AI, rendering) and rewrite them as systems that query the registry.
4. **Port the Arena**: The `Arena` class is actually already quite data-driven in spirit (the grid is, after all, pure data). It mostly needs to learn to talk to the `registry`. 
5. **External Config**: move hardcoded values (speeds, sizes, colors, arena layouts) into `JSON` files. Systems read config at startup.

#### The Starting Point:
Before writing a single game-specific line of code, we need a working `Registry`. That's the heart of this whole new thing, and a minimal one needs to:
1. Create and destroy entities
2. Add/remove components to entitities
3. Query "give me all entities with components X, and Y, and Z, and..."

> Also, a side note, the refactoring/remaking of the whole project is going to also co-locate definition and implementation files, i.e. `.hpp` and `.cpp` files are going to be stored alongisde in their new, corresponding directories. Apparently, this is a more modern approach than the (C-inherited, really) `incs/`/`srcs/` way of structuring a project, a tendency, even, so let's adhere to it. Because why not.

### 3.3.1 The ECS Foundation implementation
Three files. That's all the foundation is: `Entity.hpp`, `ComponentPool.hpp` and `Registry.hpp` + `Registry.cpp`. Let's down each one.

#### `Entity.hpp`: The Simple ID

```cpp
#pragma once

#include <cstdint>

class Entity {
public:
    using ID = std::uint32_t;

    explicit Entity(ID id) : id(id) {}

    ID getID() const { return id; }

    bool operator==(const Entity& other) const { return id == other.id; }
    bool operator!=(const Entity& other) const { return id != other.id; }

private:
    ID id;
};
```

Genuinely the most simple class I've ever written. It's just a `uint32_t` dressed up in a struct. The things worth noting:
- The constructor is `explicit`, which prevents the compiler from silently converting raw integers into entities. Without this, `Entity e = 5;` would compile without a complaint, which would be a sad thing, not to mention a bug.
- `operator==` and `operator!=` are necessary for `std::remove` to work in the registry's `destroyEntity`. Without them, the compiler has no way of comparing two entities when it tries to find the one to remove from the list.
- `using ID = std::uint32_t` gives a name to the underlying type. This is useful in `ComponentPool`, which needs to use the same type for the map key, and instead of hardcoding `uint32_t` there too, it can just reference `Entity::ID`. One source of truth for the type, which I've been told is a habit worth building.

#### `ComponentPool.hpp`: The New Storage

```cpp
template<typename T>
class ComponentPool {
public:
    void add(Entity entity, T component);
    T& get(Entity entity);
    const T& get(Entity entity) const;
    bool has(Entity entity) const;
    void remove(Entity entity);

private:
    std::vector<T>                          components;    // packed, cache-friendly
    std::unordered_map<Entity::ID, size_t>  entityToIndex; // entity ID -> index in components
};
```

This is where the cache-friendliness mentioned in section 3.2 actually lives. The design is:
- One `ComponentPool<T>` exists per component type. All `PositionComponent`s live in one pool, all `MovementComponent`s in another. Never mixed.
- `components` is a packed `std::vector<T>`. No gaps, no nullptrs, no indirection. Just the data, lined up.
- `entityToIndex` maps an entity's ID to its index in `components`. This is how we go from "I want Snake A's position" to "position is at index 3 in the array".

The most interesting part is `remove()`. Naively, removing from the middle of a vector means either leaving a gap (bad, breaks the packing) or shifting everything after it (slow). The solution: **swap with the last element, then pop**:

```cpp
void remove(Entity entity) {
    const size_t removedIndex = entityToIndex[entity.getID()];
    const size_t lastIndex    = components.size() - 1;

    if (removedIndex != lastIndex) {
        components[removedIndex] = components[lastIndex];
        // Find who owned the last slot and update their index
        for (auto& [eid, idx] : entityToIndex)
            if (idx == lastIndex) { idx = removedIndex; break; }
    }

    components.pop_back();
    entityToIndex.erase(entity.getID());
}
```

Order in the vector is not preserved, but order doesn't matter here. What matters is that the array stays packed and that `pop_back()` is O(1). The only cost is the loop to find who owned the last slot, which is a minor thing at game scale.

One thing that was *not* done here: a `Component` base class. An earlier draft of this file had one, with a virtual destructor and everything, inherited by all components. That's an OOP reflex that needs (I need) to be consciously unlearned (by me). Components are pure data. No base class, no virtual anything, no inheritance. If a component starts having methods and a hierarchy, it's no longer a component, it's a class, and we're back to OOP.

#### `Registry.hpp` + `Registry.cpp`: The Glue

The registry is what everything else talks to. Entities are created here, components are added here, and systems query here. Its private layout:

```cpp
private:
    std::vector<Entity>                                         entities;
    std::unordered_map<std::type_index, std::shared_ptr<void>> componentPools;
    size_t                                                      entityCount = 0;
```

- `entities` is just a list of all living entities.
- `componentPools` maps a component type (via `std::type_index`) to its pool. The pool is stored as `shared_ptr<void>` because the registry needs to hold pools of different types in the same map, and C++ doesn't let you have a `map<type_index, ComponentPool<?>>` without type erasure. The actual typed pointer is recovered via `static_pointer_cast<ComponentPool<T>>` whenever a typed operation is needed.
- `entityCount` is a monotonically increasing counter. Each new entity gets the next ID. Simple and sufficient for now (no recycling of destroyed IDs yet, but that's a future concern).

The public interface resolves to five operations: `createEntity`, `destroyEntity`, `addComponent<T>`, `getComponent<T>`, `removeComponent<T>`, and the querying method, `view<T...>()`:

```cpp
template<typename... T>
std::vector<Entity> Registry::view() const {
    std::vector<Entity> result;
    for (const auto& entity : entities) {
        if ((hasComponent<T>(entity) && ...))
            result.push_back(entity);
    }
    return result;
}
```

The `(hasComponent<T>(entity) && ...)` part is a **C++17 fold expression**: the compiler expands it at compile time into a chain of `&&` checks, one per type in the pack. So `view<PositionComponent, MovementComponent>()` expands to `hasComponent<PositionComponent>(entity) && hasComponent<MovementComponent>(entity)`. Short-circuits on the first `false`. This is what systems will call every frame to get their working set of entities, which means it needs to be fast. For the scale of this game, iterating over the full entity list is fine. A more advanced registry would maintain per-component bitsets to make this a bitwise AND operation, but that's something to be implemented *if* necessary down the line.

The non-template bits — `createEntity`, `destroyEntity`, and `getEntities` — live in `Registry.cpp`:

```cpp
Entity Registry::createEntity() {
    Entity entity(static_cast<Entity::ID>(entityCount++));
    entities.push_back(entity);
    return entity;
}

void Registry::destroyEntity(Entity entity) {
    auto it = std::remove(entities.begin(), entities.end(), entity);
    entities.erase(it, entities.end());
}
```

The `static_cast<Entity::ID>` in `createEntity` is required because the `Entity` constructor is `explicit`. Everything template-related stays in the header: this is non-negotiable with C++ templates, since the compiler needs to see the full template definition at the point of instantiation, not just a declaration. Splitting them like some earlier drafts attempted would have resulted in linker errors that, while eventually diagnosable, are the kind of thing that idnaps hours of your life.

With this, the ECS core is complete. Three files, a handful of concepts, and now the rest of the engine has somewhere to stand. And I shall move on to start writing components.

<br>
<br>

## 3.4 Carefully Composing Components
Nice aliteration. Anyway, we can start thinking about how to write the component structure and the components themselves by thinking about the core-most entity of a *Snake* game. You guessed it right again, the `snake`, which remember, it's no longer a class in itself, just **an entity (i.e., an ID) with a collection of components attached to it, including a `SnakeComponent`**, this specific one being the one that conceptually defines it as a snake (I guess??? Maybe this thought should be tossed away completely when trying to get rid of OOP thoughts in favor of DD feelings). So, what would a snake entity's components be? (*Since the question for the snake component is going to be the more complex one, we'll leave it for last*)
- `PositionComponent`: this one is instant and unambiguous, as **every entity that lives on the grid needs a position (X-Y)**.
- `RenderComponent`: kind of self-explanatory, too, as everything that needs to be displayed in game needs information about how to be drawn, like color, visual state flags, etc
- `MovementComponent`: direction, move timer and move interval information. Remember, the *act* of moving will be managed by a system, that will be its job, so what this component holds is just the state needed to *perform* a move.
- `InputComponent`: buffered input container taken from player interaction
- `AIComponent`: path, target, behavior state of an AI controlle snake
	- *Note: the component structure makes some thing "easier" to manage, at least to immediately read: a snake either has an `InputComponent` or an `AIComponent`, never both. This way, systems can cleanly work on them.*

Now, from this point, two different things need to be considered: **how to handle collision information** and **how to constitute a snake**. Regarding the former, my immediate idea was to make a `CollisionComponent`, but if you think about it... **What would be the information inside such collision component?** Let's put ourselves in situation: a snake is happily frolicking around, eating apples, and its head collides into a wall or an obstacle. The collision itself, at least up until this point, is defined by position superposition, i.e. if the head's XY values are the same as a wall/obstacle cell, collision happened (and right now, death follows). So, then again, when that collision is detected... Is there any info to be parsed? After some consideration, at this very moment my answer would be no (or maybe with some tint of question, something like "no??"; I'm insecure like that).

Then, what does that mean for our ECS dreams and hopes? Nothing to worry about! We'll just have an empty struct, kind of a way to just detect that something *should* trigger a collision, some sort of... **TAG**! And lo and behold, this is something that is apparently usual in ECS approaches, **Tags that give statuses to entities just o that they are caught by systems**. In our case, the most straight forward thing you could ever imagine:
```cpp
struct SolidTag {};
```

And that's it! Anything that collides is, by definition here, *solid*! So by having the `SolidTag`, a `PhysicsSystem` (or `CollisionSystem`, we'll see) will fish it out of the pool and act on it. Very nice!

On the snake side of things, things are obviously not going to be that simple. Some d e c i s i o n s need to be made, some scrutiny needs to mediate between us and the cosmos (the code). So, how to think about the snale? What information would go into an hypothetical `SnakeComponent`? We could think about something along the line of a **list of body segments**, but this comes with second degree questions: **Is the body a single component? Or should each segment be its own entity?**. We could lay out two main ways of going around it:
- **Body as data inside one component**: much simpler, based on a `SnakeComponent` holding an `std::vector<Vec2> segments` list. One entity per snake. Easier to reason about for a small cale game like what we're initially building. 
- **Each segment as its own entity**: much purer ECS, with each segment being an entity itself with a `PositionComponent` and a `SegmentComponent` that links it to its parent snake. More flexible, but tickling that sixth sense that lights up when it feels an overkill lurking in the shadows.

Instinct says: for what we're building, **one entity per snake, body stored inside the compoent** is the right call. A `SnakeComponent` holding the segment list, the growing flag, maybe the current length. BUT WAIT, there's more to consider, because we're in the middle of a rebuild, but further plans are still laid out, specifically those regarding the **bead mechanics**. So knowing that, we should try to avoid any decision that gets in the way of it in the future, hich calls for a deeper preview of the two possible models

#### Model A: Bead as component on segment entities
This would mean splitting the snake body into **per-segment entities**, each with its own components.
```
Entity 5  →  PositionComponent, SegmentComponent(ownerSnake=0), BeadComponent(type=Shield)
Entity 6  →  PositionComponent, SegmentComponent(ownerSnake=0), BeadComponent(type=Speed)
Entity 7  →  PositionComponent, SegmentComponent(ownerSnake=0)   // plain segment, no bead
```
This model has advantages like it being **maximally composable**, as each bead type can be its own tag or data component, and adding/removing bead statuses at runtime would be trivial. But it will also mean **more entities to manage**, with `SnakeComponent` needing to track a list of entity `ID`s instead of `Vec2` positions, which would result in a slightly more complex movement and rendering logic.

#### Model B: Bead data inside `SnakeComponent`
Taking this route, **the snake remains one entity**. `SnakeComponent` holds the segment list, and each entry carries an optional bead type. Something like:
```cpp
enum class BeadType { None, Shield, Speed, Ghost, ... };

struct Segment {
	Vec2 position;
	BeadType bead = BeadType::None;
};

struct SnakeComponent {
	std::vector<Segment> segments;
	bool growing = false;
};
```
This clearly is a more simpler approach, with one entity per snake, barely changing movement and collision logic. But bead logic itself would end up inside systems that already handle snake movement (for example), which feels like **concern blending**. So how to decide?

Well, let's keep putting questions on top of the table: **do beads need to interact with systems independently of the snake?**
- If a `ShieldBead` should absorb a collision that would otherwise kill the snake, for example, the collision system needs to know about it. That logic can live in the collision system reading from `SnakeComponent::segments[i].bead` just as easily as from a `BeadComponent` on a separate entity.
- But if beads start having their own **timers, animations, cooldowns, cascading effects**... Things that are fundamentally per-bea state rather than per-snake state... Then Model A starts earning its complexity cost.

Yeah, at this point this just sounds like me fighting against the demons of complexity, but for a first implementation I think that Model B is the sane approach. It's simpler, nothing is really lost and it maps cleanly to how the game actually will think about snakes (**one snake, one entity, body is internal structure**). I can always migrate to model A later if bead complexity demands it, and one of the tradeoffs of all this transition is that said migration would be quite easier because the systems will we fully decoupled. So why deal with a per-bead complexity layer before even having beads, first, and coming to the conclusion that they need their component independence, later?

In conclusion: **Model B with `BeadType` as an `enum class` in its own header, code-wise separated from `SnakeComponent`. This way, if the bead logic grows it has a natural home to expand into without touching the snake component itself. We'll see if future-me ends up hating me :D

#### A Snake's Worth of Components

<br>
<br>

## 3.5 Writing the Systems

With components defined and the registry working, it was time for the other half of the ECS equation: **systems**. Five of them, covering the full game loop. The update order matters: `Input → AI → Movement → Collision → Render`. Each one must run in that sequence every frame, and each one only knows about the registry — no direct references between systems.

### `InputSystem` — Catching Keypresses

```cpp
class InputSystem {
public:
    void assignSlot(Entity entity, PlayerSlot slot);
    void update(Registry& registry);
private:
    std::unordered_map<Entity::ID, PlayerSlot> slotMap;
    void pollPlayerA(Registry& registry, Entity entity);
    void pollPlayerB(Registry& registry, Entity entity);
};
```

The input system's job is narrow: poll Raylib for keypresses each frame and push them into the relevant entity's `InputComponent::inputBuffer` queue. It knows nothing about movement, nothing about what happens with those inputs — it just records them.

A key design decision here: the system maintains an internal `slotMap` that associates entity IDs to `PlayerSlot` (A or B). When `update()` iterates over `view<InputComponent>()`, it skips any entity that hasn't been assigned a slot. This is how AI-controlled snakes stay out of the input pipeline — they simply have no slot. No flags, no special cases in the logic, just an absence of a map entry.

The input buffer is a `std::queue<Input>` on the component. Pushing to the queue here and consuming from it in `MovementSystem` means that if the player presses two keys in the same frame (which can and does happen), neither is lost. The movement system will process them one per tick. This is the classic way of making snake input feel responsive without allowing the player to reverse 180° by pressing two keys in rapid succession.

### `MovementSystem` — The Two-Phase Tick

```cpp
void MovementSystem::update(Registry& registry, float deltaTime) {
    processInput(registry);
    advanceSnake(registry, deltaTime);
}
```

Movement splits into two phases: direction update, then position advance.

**`processInput`** reads one input from each snake's buffer, validates it (no 180° reversals — checked by summing the current and requested direction vectors; if they cancel to zero, it's a reversal), and updates `MovementComponent::direction`. The reversal guard is:

```cpp
const bool isReversal = (currentVec.x + requestedVec.x == 0) &&
                         (currentVec.y + requestedVec.y == 0);
```

Simple and branchless. A snake moving right has `currentVec = {1, 0}`, and requesting left gives `requestedVec = {-1, 0}`. They sum to zero — reversal detected, input discarded.

**`advanceSnake`** is where the snake actually moves. The algorithm:

```cpp
snake.segments.push_front({ newHead, BeadType::None });
pos.position = newHead;

if (snake.growing) {
    snake.growing = false;
} else {
    snake.segments.pop_back();
}
```

`segments` is a `std::deque<Segment>`. `push_front` + `pop_back` are both O(1), and they preserve the length of the snake exactly: add a head, remove a tail, net change of zero. When the snake has just eaten, `growing` is `true` (set by `CollisionSystem` the previous frame), so `pop_back` is skipped and the snake gains one segment.

One thing that burned some time here: an earlier version of this method shifted segment positions in-place in a loop, then called `pop_back`. The "shift" was updating position values inside existing `Segment` elements — but it never actually added a new element at the head. So the deque size decreased by one on every tick, and the snake melted. The fix was conceptually simple but easy to miss: **push a new element, then pop the old tail**. Don't try to simulate insertion by shifting values.

A second thing worth noting: `moveTimer` resets to `0.0f` (not `moveTimer -= moveInterval`). The subtractive approach lets residual time carry over to the next tick, which sounds right in theory but in practice causes multi-tick bursts when there's a frame time spike (e.g., during window initialization). Resetting cleanly to zero means one move per timer expiry, no exceptions.

### `CollisionSystem` — Priority Queue of Deaths

The collision system has a clear job: after movement has resolved, check if any snake's head is now occupying a bad cell, and record what it hit in `CollisionResultComponent`. It doesn't do anything *about* the collision — that's for the game loop to decide. It just writes a result.

The check order is intentional priority:

```
clearResults → Wall → Self → Snake → Food
```

Each check short-circuits if a result is already written. So if a snake hits a wall and its own body simultaneously (possible at corners), wall wins. The reason to care about this order becomes obvious once you think about game logic: you don't want "food eaten" to fire when the snake is also dead.

The wall check is the most interesting performance-wise. It builds a flat `std::vector<Vec2>` of all solid positions once, then iterates snakes against it. Doing a `view<SolidTag>()` per snake entity would mean re-fetching the same solid list for each snake. By gathering them once first, it's one registry query instead of N.

Food collision is the one side effect this system is allowed: it sets `snake.growing = true` directly on the `SnakeComponent`. Everything else — food relocation, score updates, game over — is left to the game loop reading the `CollisionResultComponent` each frame.

### `AISystem` — The Decision Hierarchy

The AI system is the most involved one. It decides what direction an AI-controlled snake should move and writes it directly into `MovementComponent::direction`. No input buffer — AI doesn't go through the input queue.

The decision hierarchy for each AI entity, in order:

1. **Easy mode random roll**: if configured, occasionally skip food-hunting entirely and just maximize space.
2. **`goToFood`**: A* pathfind to the nearest food. If the path is found but the safety check (flood fill reachability from the endpoint) fails, fall into survival mode.
3. **`survivalMode`**: A* pathfind to the snake's own tail. The tail is always a valid target because it will have moved by the time the head arrives. If unreachable, fall into maximize space.
4. **`maximizeSpace`**: check all four cardinal neighbors, flood-fill each to count reachable cells, pick the direction with the most space.

The biggest performance issue in the original implementation: `goToFood`, `survivalMode`, and `maximizeSpace` all called `isWalkable(registry, ...)` per node during pathfinding and flood fill. `isWalkable` queried `view<SolidTag>()` and `view<SnakeComponent>()` — i.e., two full entity-list scans and two heap allocations per *node* during A*. On a 31×31 grid with `maxDepth=200`, a single AI decision could trigger thousands of allocations per frame. The game ran at something like 2-3 moves per second.

The fix: **build the blocked grid once per AI entity per update, pass it everywhere**.

```cpp
void AISystem::update(Registry& registry) {
    for (auto entity : registry.view<AIComponent, ...>()) {
        const BlockedGrid blocked = buildBlockedGrid(registry);
        Direction dir = decideDirection(registry, entity, blocked);
        registry.getComponent<MovementComponent>(entity).direction = dir;
    }
}
```

`buildBlockedGrid` does two registry queries and fills a `vector<vector<bool>>[x][y]`. Everything downstream — A*, flood fill, `isSafeMove` — does direct `blocked[x][y]` array access. Zero allocations, zero hash map lookups inside the search loops. The same performance fix was applied to `GridHelper::isWalkable`, `Pathfinder::findPath`, and both `FloodFill` methods, which all now take the pre-built grid by const reference instead of a registry.

At the same time, `getNeighbors` was updated to filter out-of-bounds positions at the point of generation (it now takes `gridWidth` and `gridHeight`), rather than letting the caller handle boundary checking after the fact.

### `RenderSystem` — Two Pipelines

The render system is the only one that touches Raylib directly. It exposes a single entry point:

```cpp
void render(Registry& registry, RenderMode mode, float deltaTime);
```

`RenderMode` is an enum (`MODE2D` / `MODE3D`). The 2D and 3D pipelines are completely separate internal methods — no shared draw calls, no conditional branching inside draw functions.

**2D pipeline**: flat grid, square cells, centered arena. The layout is calculated at `init()` time from `gridWidth`, `gridHeight`, `squareSize`, and `borderThickness`. `gridToScreen2D(x, y)` converts grid coordinates to pixel coordinates for any given cell. Food gets a sine-wave pulse driven by `accumulatedTime`:

```cpp
float pulse = 1.0f + sinf(accumulatedTime * 3.0f) * 0.1f;
float size  = static_cast<float>(squareSize) * 0.7f * pulse;
```

**3D pipeline**: isometric orthographic view using Raylib's `Camera3D`. Camera position is derived from grid diagonal to ensure the whole arena fits in frame regardless of arena size, using a quadratic fit for the FOV value that was calibrated to match a visually comfortable frame at various sizes. The checkerboard ground plane, walls, food, and snakes all use `drawCubeCustomFaces` — a low-level function that draws a cube by submitting six quads directly via `rlgl`, one per face, each with its own color. This preserves the per-face shading from the old OOP renderer, which Raylib's built-in `DrawCube` can't do since it applies a single color to all faces.
