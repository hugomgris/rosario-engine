## Todo list
- [ ] Documentation
- [ ] Linkedin post
- [x] Recover tests
- [ ] cleanup main.cpp
- [ ] Fix head-to-head collisions, which make the game crash
- [ ] Color switching following mode in main menu
- [ ] Extra text info in start and gameover screens


## Test Suite (GTest-based)

### Unit Tests - Core ECS
- [ ] Registry: Multiple component types on single entity
- [ ] Registry: Component pool isolation (one component type doesn't affect another)
- [ ] Registry: Entity destruction and cleanup (no dangling references)
- [ ] Registry: hasComponent() returns false after removeComponent()
- [ ] Registry: forEach() with single component type
- [ ] Registry: view() with multiple component filters
- [ ] Registry: Attempting to get non-existent component throws exception
- [ ] Registry: Accessing destroyed entity throws exception
- [ ] ComponentPool: Capacity growth under stress (many entities)
- [ ] ComponentPool: Copy and move semantics for components

### Unit Tests - Data Loaders
- [ ] ParticleConfigLoader: All sections parse correctly (dust, explosion, trail, menuTrail*)
- [ ] ParticleConfigLoader: ParticleConfig color parsing (array and object formats)
- [ ] ParticleConfigLoader: Direction enum parsing (UP/DOWN/LEFT/RIGHT)
- [ ] ParticleConfigLoader: Malformed JSON throws exception
- [ ] ParticleConfigLoader: Missing required fields throw exception
- [ ] ParticleConfigLoader: Out-of-range values throw exception
- [ ] AIPresetLoader: Parse AI presets with difficulty levels
- [ ] AIPresetLoader: AIPreset data structures match config
- [ ] AIPresetLoader: Invalid difficulty level throws exception
- [ ] ArenaPresetLoader: Parse arena preset configurations
- [ ] ArenaPresetLoader: Wall matrix dimensions valid
- [ ] CollisionRuleLoader: Parse collision rules from JSON
- [ ] CollisionRuleLoader: Subject/object pairs correctly stored
- [ ] CollisionRuleLoader: Malformed rules throw exception
- [ ] TunnelConfigLoader: Parse tunnel animation config
- [ ] PostProcessingConfigLoader: Parse post-processing effect settings
- [ ] All Loaders: Missing config file throws appropriate exception
- [ ] All Loaders: Corrupted JSON throws parsing exception

### Unit Tests - AI Pathfinding & Grid
- [ ] GridHelper: Grid coordinate validation (bounds checking)
- [ ] GridHelper: Neighbor generation for interior cell
- [ ] GridHelper: Neighbor generation at grid edges
- [ ] GridHelper: Neighbor generation at grid corners
- [ ] GridHelper: Manhattan distance calculation
- [ ] FloodFill: Count reachable cells on empty grid
- [ ] FloodFill: Count reachable cells with obstacles
- [ ] FloodFill: Count reachable from various start positions
- [ ] FloodFill: Properly ignores specified positions
- [ ] FloodFill: Detect unreachable areas (isolated regions)
- [ ] FloodFill: Zero reachable cells when completely surrounded
- [ ] FloodFill: canReachTail() returns true when tail reachable
- [ ] FloodFill: canReachTail() returns false when tail blocked
- [ ] Pathfinder: Find path on empty grid (straight line)
- [ ] Pathfinder: Find path with single obstacle (circumnavigate)
- [ ] Pathfinder: Find path with complex obstacle maze
- [ ] Pathfinder: No path returns empty vector
- [ ] Pathfinder: Respects maxDepth limit
- [ ] Pathfinder: Path is shortest (or near-optimal with A*)
- [ ] Pathfinder: Properly ignores specified positions
- [ ] Pathfinder: Start equals goal returns path of length 1
- [ ] Pathfinder: No backtracking in returned path
- [ ] GridHelper: Diagonal movement (if applicable to algorithm)

### Unit Tests - Arena
- [ ] ArenaGrid: Constructor initializes grid with correct dimensions
- [ ] ArenaGrid: setCell/getCell basic get/set operations
- [ ] ArenaGrid: getCell bounds check (returns error/exception for out-of-bounds)
- [ ] ArenaGrid: isWalkable() returns correct value for each CellType
- [ ] ArenaGrid: getAvailableCells() returns only Empty cells
- [ ] ArenaGrid: getAvailableCells() count matches expected empty cell count
- [ ] ArenaGrid: clearCell() sets cell to Empty
- [ ] ArenaGrid: clearArena() empties all cells
- [ ] ArenaGrid: spawnObstacle() creates rectangular obstacle
- [ ] ArenaGrid: growWall() adds wall to grid
- [ ] ArenaGrid: transformArenaWithPreset(InterLock1) layout is correct
- [ ] ArenaGrid: transformArenaWithPreset(Spiral1) layout is correct
- [ ] ArenaGrid: transformArenaWithPreset(Columns1) layout is correct
- [ ] ArenaGrid: transformArenaWithPreset(Maze) layout is correct
- [ ] ArenaGrid: getAllOutlines() returns correct outline vectors
- [ ] ArenaGrid: beginSpawn() initiates spawn timer
- [ ] ArenaGrid: setMenuArena() vs setGameplayArena() produce different layouts
- [ ] ArenaGrid: Wall preset boundaries (perimeter stays intact)

### Unit Tests - Collision System
- [ ] CollisionRule struct construction and field assignment
- [ ] CollisionRuleTable::find() returns correct rule for subject/object pair
- [ ] CollisionRuleTable::find() returns nullptr for unknown pair
- [ ] CollisionRuleTable::find() order-independent (subject ≠ object)
- [ ] CollisionEffectDispatcher::dispatchEffect() applies correct effects
- [ ] CollisionEffectDispatcher: Multiple effects on single collision
- [ ] CollisionEffectDispatcher: Invalid effect name throws/logs error

### Unit Tests - Components
- [ ] SnakeComponent: Add segments to snake
- [ ] SnakeComponent: Remove segments from snake
- [ ] SnakeComponent: Segment count matches constructor/modifications
- [ ] SnakeComponent: Powerup bead assignment (Shield, Speed, Ghost, Ram)
- [ ] SnakeComponent: BeadType enum values valid
- [ ] PositionComponent: Grid coordinates within bounds
- [ ] PositionComponent: Coordinate getters return correct values
- [ ] ScoreComponent: Increment score correctly
- [ ] ScoreComponent: Score cannot go negative
- [ ] MovementComponent: Direction enum assignment (UP/DOWN/LEFT/RIGHT)
- [ ] MovementComponent: Prevent opposite direction moves (UP ↔ DOWN, LEFT ↔ RIGHT)
- [ ] InputComponent: Input queue operations
- [ ] RenderComponent: Sprite/texture assignment
- [ ] RenderComponent: Tint color assignment

### Unit Tests - Configuration Structs
- [ ] ParticleConfig: All sub-configs accessible
- [ ] ParticleConfig: MenuTrailPreset vector operations
- [ ] MenuTrailPreset: Color and direction assignment
- [ ] MenuTrailPreset: Manual position (x, y) handled correctly
- [ ] ParticleConfig: Default values applied correctly

---

## Integration Tests - ECS System
- [ ] Registry: Create 100+ entities with mixed components
- [ ] Registry: Query with multiple component types returns correct entities
- [ ] Registry: forEach() on empty pool
- [ ] Registry: forEach() on pool with one entity
- [ ] Registry: View after adding/removing entities updates correctly
- [ ] Registry: Multiple views with different component sets work independently

### Integration Tests - Config Loading Pipeline
- [ ] Load ParticleConfig.json: All top-level sections populated
- [ ] Load ParticleConfig.json: MenuTrail presets count and values correct
- [ ] Load AIPreset: Difficulty settings accessible and correct
- [ ] Load ArenaPreset: Wall matrices are valid and proper dimensions
- [ ] Load CollisionRules: Rule table fully populated
- [ ] Load all configs simultaneously (no interference)
- [ ] Config reload: Old data properly replaced
- [ ] Partial config load (subset of fields): Non-specified fields use defaults

### Integration Tests - Pathfinding + Arena
- [ ] Pathfind on InterLock1 arena layout
- [ ] Pathfind on Spiral1 arena layout
- [ ] Pathfind on Maze arena layout
- [ ] Pathfinder output walkable on actual arena (all cells empty/walkable)
- [ ] FloodFill on generated arena matches pathfinder reachability
- [ ] FloodFill countReachable() includes all cells reachable by pathfinder
- [ ] Arena with spawn/despawn obstacles: Pathfind avoids moving obstacles
- [ ] Pathfinding performance: Find path in 100×100 grid under time limit

### Integration Tests - Collision System
- [ ] Load collisionRules.json into CollisionRuleTable
- [ ] Add snakes to registry, query by collision rule
- [ ] Dispatch collision effect chain (multiple effects from one collision)
- [ ] Collision with different pairs (Snake-Food, Snake-Wall, Snake-Snake)
- [ ] No collision when entities don't meet conditions

### Integration Tests - Game State & Entity Lifecycle
- [ ] MenuArena: Create arena, load wall preset, verify no walkable cells for gameplay
- [ ] MenuArena → GameplayArena: Transition creates proper arena layout
- [ ] Create player snake with full components (Position, Render, Movement, Snake)
- [ ] Create AI snake with full components (Position, Render, Movement, AI, Snake)
- [ ] Create food entity and verify it's renderable
- [ ] Create button UI entities and verify layout
- [ ] Destroy entity: All components properly cleaned up

### Integration Tests - AI + Pathfinding + Movement
- [ ] AI system generates path on arena
- [ ] Path generated is walkable on arena
- [ ] Movement system applies first direction from AI path
- [ ] Snake moves along computed path over multiple frames
- [ ] AI recalculates path when blocked
- [ ] AI prefers longer safe path over short dangerous path (if applicable)

### Integration Tests - Particle System + Config
- [ ] Load ParticleConfig with menu trails
- [ ] Enqueue multiple ParticleSpawnRequests
- [ ] Particle system respects spawnInterval (time-gated emission)
- [ ] Menu trails emit from configured positions (x, y)
- [ ] Trail particles use configured color and direction
- [ ] Multiple trail types (yellow, blue) emit independently
- [ ] Dust particles emit with correct density and size
- [ ] Explosion particles spawn with correct count and scatter

### Integration Tests - Input → Movement → Rendering
- [ ] InputSystem processes direction input
- [ ] MovementComponent receives direction from input
- [ ] MovementSystem applies direction to SnakeComponent
- [ ] RenderSystem renders snake at updated position
- [ ] Invalid moves (opposite direction) are rejected
- [ ] Input queue processes multiple frames of buffered input

### Integration Tests - Render System
- [ ] Load all image assets and verify non-null
- [ ] Render arena with walls (getAllOutlines generates valid geometry)
- [ ] Render single snake segment
- [ ] Render multi-segment snake with correct ordering
- [ ] Render food at correct position
- [ ] Render UI buttons in correct positions
- [ ] Render particles with correct lifetime and size

### Integration Tests - Complete Frame Updates
- [ ] Single frame: Load config → Create entities → Update all systems → No crash
- [ ] 100 frames: Game loop stability test
- [ ] Rapid input: Queue multiple directions, verify correct sequence applied
- [ ] Snake collision with wall: Movement stopped, score/health affected
- [ ] Snake collision with food: Score increased, new food spawned
- [ ] Snake collision with self: Game over triggered
- [ ] Snake collision with AI snake: Collision effect applied (if applicable)
- [ ] Post-processing effect toggle: Visual effect applies/removes correctly
- [ ] Tunnel animation plays correctly on configured frames

### Integration Tests - Memory & Performance
- [ ] Valgrind: No memory leaks during full game loop (100+ frames)
- [ ] Valgrind: No invalid heap accesses during gameplay
- [ ] Load and unload config repeatedly: Memory stable
- [ ] Create and destroy 1000 entities: Proper cleanup
- [ ] Arena transform timing: All wall presets apply without stall
- [ ] Large pathfind (150×150 grid): Completes in < 100ms

### Integration Tests - Edge Cases & Error Recovery
- [ ] Invalid config format: Graceful error message, game continues
- [ ] Missing asset file: Graceful fallback or error, no crash
- [ ] Corrupted save/preset: Recovery attempted, sensible defaults applied
- [ ] Snake out of bounds (shouldn't happen): Safely clamped or wrapped
- [ ] Extreme grid sizes (1×1, 500×500): Handled without crash
- [ ] Zero playable cells in arena: Detected and handled gracefully

---

### Long Run
- Entity ID recycling/reusing