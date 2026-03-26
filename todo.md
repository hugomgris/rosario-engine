## Todo list
- [x] Documentation
- [ ] Linkedin post
- [x] Recover tests
- [x] cleanup main.cpp
- [ ] Fix head-to-head collisions, which make the game crash
- [ ] Color switching following mode in main menu
- [ ] Extra text info in start and gameover screens


## Test Suite (GTest-based)

### Unit Tests - Core ECS
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

### Unit Tests - Data Loaders
- [x] ParticleConfigLoader: All sections parse correctly (dust, explosion, trail, menuTrail*)
- [x] ParticleConfigLoader: Multiple menuTrail entries
- [x] ParticleConfigLoader: ParticleConfig color parsing (array and object formats)
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

### Unit Tests - AI Pathfinding & Grid
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

### Unit Tests - Arena
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

### Unit Tests - Collision System
- [x] CollisionRule struct construction and field assignment
- [x] CollisionRuleTable::find() returns correct rule for subject/object pair
- [x] CollisionRuleTable::find() returns nullptr for unknown pair
- [x] CollisionEffectDispatcher::dispatchEffect() applies correct effects
- [x] CollisionEffectDispatcher: Multiple effects on single collision
- [x] CollisionEffectDispatcher: Invalid effect name throws/logs error

### Unit Tests - Components
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

### Unit Tests - Configuration Structs
- [x] ParticleConfig: All sub-configs accessible
- [x] ParticleConfig: MenuTrailPreset vector operations
- [x] MenuTrailPreset: Color and direction assignment
- [x] MenuTrailPreset: Manual position (x, y) handled correctly
- [x] ParticleConfig: Default values applied correctly

---

## Integration Tests

#### Config + Runtime Wiring
- [ ] Boot loads all runtime JSON files together and binds them to systems (no cross-loader interference)
- [ ] Hot reload glyph presets updates visible UI text entities without restarting
- [ ] Hot reload fallback path works when a preset is removed (template fallback still renders)
- [ ] Reloading particle config updates menu trail behavior on next frame (direction/color/interval)

#### UI + Pixel Text + Menu Particles
- [ ] Menu keyboard/mouse co-navigation preserves single hover source of truth across frames
- [ ] State-scoped pixel text visibility toggles correctly between Menu and GameOver
- [ ] Menu logo trail requests follow text visibility and stop when logo is hidden
- [ ] Multiple menuTrail emitters run independently with per-emitter interval gating

#### Rendering and Frame Pipeline (Smoke)
- [ ] One full frame in each state (Menu, Playing, GameOver) runs update+render with no crashes

#### State Transitions and World Rehydration
- [ ] Menu -> Playing transition resets world and spawns expected gameplay entities
- [ ] Playing -> GameOver transition swaps UI layout and state-visible text correctly
- [ ] ReturnToMenu action clears gameplay entities and restores menu entities cleanly
- [ ] Registry reset during transition does not leave stale references in UI/pixel text helpers
- [ ] Re-entering Playing multiple times does not duplicate persistent entities

#### Input -> Decision -> Movement Chain
- [ ] Player input buffering is consumed across frames in deterministic order
- [ ] Opposite-direction rejection remains correct across multiple buffered inputs
- [ ] VS-AI mode: AI decision output is reflected in movement on the next tick

#### Collision -> Effects -> Gameplay Outcome
- [ ] Snake-Food collision applies full effect chain (grow + score + relocate food)
- [ ] Snake-Wall collision transitions gameplay to death/gameover path
- [ ] Snake-Self collision transitions gameplay to death/gameover path
- [ ] Multi-effect collision dispatch is idempotent per tick (no double application)

#### Arena + AI Coherence
- [ ] Arena preset application is reflected in AI blocked-grid view within same update cycle
- [ ] Path selected by AI remains walkable after movement step validation
- [ ] Flood-fill safety check and pathfinder decision stay consistent on identical board state
- [ ] AI repaths after dynamic arena changes (spawn/despawn solids) without freezing

#### Rendering and Frame Pipeline
- [ ] Mode2D/Mode3D switch during gameplay preserves render/update coherence
- [ ] Post-processing on/off toggle does not break UI composition order
- [ ] Tunnel animation and particle rendering coexist without state leakage

#### Collision Edge Case (Current Known Risk)
- [ ] Head-to-head snake collision resolves deterministically and does not crash

#### End-to-End Soak and Stability
- [ ] 300-frame deterministic soak in Playing (fixed seed, scripted inputs) produces stable outcomes
- [ ] Repeated state cycling (Menu <-> Playing <-> GameOver) for N loops has no leaks under Valgrind
- [ ] Long-run entity churn (spawn/despawn gameplay objects across rounds) shows no invalid accesses
- [ ] Config reload + gameplay transition sequence remains stable (no crash, no stale pointers)

#### Performance Gates (Integration Level)
- [ ] 95th percentile frame time stays under target during stress scenario (particles + AI + postFX)
- [ ] AI tick time stays under target on heavy arena preset during active gameplay
- [ ] Particle system update time stays bounded with concurrent menu trail emitters

---

### Long Run
- Entity ID recycling/reusing