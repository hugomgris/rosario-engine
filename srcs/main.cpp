#include <iostream>
#include <algorithm>
#include <ctime>
#include <raylib.h>

#include "ecs/Registry.hpp"
#include "components/PositionComponent.hpp"
#include "components/MovementComponent.hpp"
#include "components/SnakeComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/AIComponent.hpp"
#include "components/RenderComponent.hpp"
#include "components/CollisionResultComponent.hpp"
#include "components/SolidTag.hpp"
#include "components/FoodTag.hpp"
#include "systems/InputSystem.hpp"
#include "systems/AISystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/TextSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/MenuSystem.hpp"
#include "systems/PostProcessingSystem.hpp"
#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include "core/GameState.hpp"
#include "../incs/DataStructs.hpp"
#include "../incs/Colors.hpp"
#include "../incs/RaylibColors.hpp"

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr int SCREEN_W = 1920;
static constexpr int SCREEN_H = 1080;

// ─── Argument validation ──────────────────────────────────────────────────────

static bool parseArguments(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s.find_first_not_of("0123456789") != std::string::npos) {
            std::cerr << "error: bad argument {" << argv[i] << "}: only numeric arguments accepted\n";
            return false;
        }
    }
    return true;
}

// ─── Entity factories ─────────────────────────────────────────────────────────

static Entity spawnPlayerSnake(Registry& registry,
                                InputSystem& inputSystem,
                                Vec2 startPos,
                                int  initialLength,
                                BaseColor color,
                                PlayerSlot slot) {
    Entity e = registry.createEntity();
    SnakeComponent snake;
    for (int i = 0; i < initialLength; ++i)
        snake.segments.push_back({ { startPos.x - i, startPos.y }, BeadType::None });
    registry.addComponent(e, snake);
    registry.addComponent(e, PositionComponent{ startPos });
    registry.addComponent(e, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
    registry.addComponent(e, InputComponent{});
    registry.addComponent(e, RenderComponent{ color });
    registry.addComponent(e, CollisionResultComponent{});
    inputSystem.assignSlot(e, slot);
    return e;
}

static Entity spawnAISnake(Registry& registry,
                            Vec2 startPos,
                            int  initialLength,
                            BaseColor color,
                            AIComponent aiConfig) {
    Entity e = registry.createEntity();
    SnakeComponent snake;
    for (int i = 0; i < initialLength; ++i)
        snake.segments.push_back({ { startPos.x - i, startPos.y }, BeadType::None });
    registry.addComponent(e, snake);
    registry.addComponent(e, PositionComponent{ startPos });
    registry.addComponent(e, MovementComponent{ Direction::RIGHT, 0.0f, 0.1f });
    registry.addComponent(e, aiConfig);
    registry.addComponent(e, RenderComponent{ color });
    registry.addComponent(e, CollisionResultComponent{});
    return e;
}

static Entity spawnFood(Registry& registry, Vec2 pos) {
    Entity e = registry.createEntity();
    registry.addComponent(e, FoodTag{});
    registry.addComponent(e, PositionComponent{ pos });
    return e;
}

static void relocateFood(Registry& registry, Entity foodEntity,
                         int gridWidth, int gridHeight,
                         const ArenaGrid* arena = nullptr) {
    std::vector<Vec2> snakeOccupied;
    for (auto e : registry.view<SnakeComponent>())
        for (const auto& seg : registry.getComponent<SnakeComponent>(e).segments)
            snakeOccupied.push_back(seg.position);

    auto isSnakeOccupied = [&](Vec2 p) {
        for (const auto& o : snakeOccupied)
            if (o.x == p.x && o.y == p.y) return true;
        return false;
    };

    if (arena) {
        auto cells = arena->getAvailableCells();
        for (int tries = 0; tries < static_cast<int>(cells.size()); ++tries) {
            Vec2 c = cells[static_cast<size_t>(std::rand()) % cells.size()];
            if (!isSnakeOccupied(c)) {
                registry.getComponent<PositionComponent>(foodEntity).position = c;
                return;
            }
        }
        return;
    }

    std::vector<Vec2> solidOccupied;
    for (auto e : registry.view<SolidTag, PositionComponent>())
        solidOccupied.push_back(registry.getComponent<PositionComponent>(e).position);

    auto isOccupied = [&](Vec2 p) {
        for (const auto& o : solidOccupied)
            if (o.x == p.x && o.y == p.y) return true;
        return isSnakeOccupied(p);
    };

    for (int attempts = 0; attempts < gridWidth * gridHeight; ++attempts) {
        Vec2 candidate = { std::rand() % gridWidth, std::rand() % gridHeight };
        if (!isOccupied(candidate)) {
            registry.getComponent<PositionComponent>(foodEntity).position = candidate;
            return;
        }
    }
}

// Resets all ECS state to a clean game ready to play.
static void resetGame(Registry& registry,
                      InputSystem& inputSystem,
                      Entity& playerSnake, Entity& aiSnake, Entity& food,
                      int gridWidth, int gridHeight,
                      ArenaGrid& arena) {
    // Wipe the entire registry and re-create everything from scratch.
    registry = Registry{};

    const BaseColor snakeAColor  = { 135, 206, 250, 255 };
    const BaseColor snakeAIColor = {  46, 179, 113, 255 };

    playerSnake = spawnPlayerSnake(registry, inputSystem,
                                   { gridWidth / 2,     gridHeight / 2 },
                                   4, snakeAColor, PlayerSlot::A);
    aiSnake     = spawnAISnake(registry,
                                { gridWidth / 2 + 6,  gridHeight / 2 },
                                4, snakeAIColor, AIComponent::medium());
    food        = spawnFood(registry, { gridWidth / 4, gridHeight / 4 });

    arena.clearArena();

    WallPreset preset = ArenaPresets::getRandomPreset();
    arena.transformArenaWithPreset(preset);
    arena.beginSpawn(1.2f);
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./rosario <width> <height>\n";
        return 1;
    }
    if (!parseArguments(argc, argv)) return 1;

    const int gridWidth  = std::stoi(argv[1]);
    const int gridHeight = std::stoi(argv[2]);

    if (gridWidth < 16 || gridHeight < 16 || gridWidth > 58 || gridHeight > 32) {
        std::cerr << "Arena must be between 16x16 and 58x32\n";
        return 1;
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ── Window ────────────────────────────────────────────────────────────────
    InitWindow(SCREEN_W, SCREEN_H, "rosario");
    SetTargetFPS(0);
    ToggleFullscreen();

    // ── Visual systems (need an open GL context) ──────────────────────────────
    TextSystem       textSystem;      textSystem.init();

    ParticleSystem   particles(SCREEN_W, SCREEN_H,
                               /*maxDust=*/60,
                               /*dustInterval=*/0.08f);

    AnimationSystem  animations;      animations.init(SCREEN_W, SCREEN_H);
    MenuSystem       menuSystem;      menuSystem.init(SCREEN_W, SCREEN_H);

    PostProcessingSystem postFX;
    postFX.init(SCREEN_W, SCREEN_H);
    postFX.setConfig(PostProcessingSystem::presetCRT());

    // Start with the menu tunnel effect
    animations.enableTunnelEffect(true, TunnelConfig::menu());

    // ── Arena ─────────────────────────────────────────────────────────────────
    ArenaGrid arena(gridWidth, gridHeight);

    // ── ECS core ──────────────────────────────────────────────────────────────
    Registry registry;

    // ── Game-play systems ─────────────────────────────────────────────────────
    InputSystem     inputSystem;
    AISystem        aiSystem(gridWidth, gridHeight);
    MovementSystem  movementSystem;
    CollisionSystem collisionSystem;
    RenderSystem    renderSystem;
    renderSystem.init(gridWidth, gridHeight);

    // ── Initial world ─────────────────────────────────────────────────────────
    Entity playerSnake(0u), aiSnake(0u), food(0u);
    resetGame(registry, inputSystem, playerSnake, aiSnake, food,
              gridWidth, gridHeight, arena);

    // ── State machine ─────────────────────────────────────────────────────────
    GameState state = GameState::Menu;
    menuSystem.setState(GameState::Menu);

    RenderMode renderMode = RenderMode::MODE2D;

    // ── Main loop ─────────────────────────────────────────────────────────────
    while (state != GameState::GameOver   // kept in loop — GameOver shows a screen
        || true) {                        // …so we need a real exit condition:
        if (WindowShouldClose()) break;

        const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);

        // Global hotkeys
        if (IsKeyPressed(KEY_F)) ToggleFullscreen();

        // ── State: Menu ───────────────────────────────────────────────────────
        if (state == GameState::Menu) {
            animations.update(dt);
            particles.update(dt);
            menuSystem.update(dt, particles);

            if (menuSystem.wantsStart()) {
                menuSystem.consumeFlags();
                state = GameState::Playing;
                animations.enableTunnelEffect(false);
            }
            if (menuSystem.wantsQuit()) {
                menuSystem.consumeFlags();
                break;
            }

            // Render ────────────────────────────────────────────────────────
            postFX.beginCapture();
            BeginDrawing();
            ClearBackground(BLACK);

            particles.render();
            menuSystem.renderMenu(textSystem, particles, &animations);

            EndDrawing();
            postFX.endCapture();
            // Note: present via postFX on next line would require not calling
            // Begin/EndDrawing twice; instead we skip full-screen postFX on
            // the menu for simplicity and render directly.
            // (Full integration done below in Playing state.)
            continue;
        }

        // ── State: Playing ────────────────────────────────────────────────────
        if (state == GameState::Playing) {
            // Dev hotkeys
            if (IsKeyPressed(KEY_TWO))   renderMode = RenderMode::MODE2D;
            if (IsKeyPressed(KEY_THREE)) renderMode = RenderMode::MODE3D;
            if (IsKeyPressed(KEY_P)) {
                arena.clearArena();
                arena.transformArenaWithPreset(ArenaPresets::getRandomPreset());
                arena.beginSpawn(1.2f);
                animations.notifyArenaSpawning();
            }

            arena.tickSpawnTimer(dt);
            arena.tickDespawnTimer(dt);

            inputSystem.update(registry);
            aiSystem.update(registry, &arena);
            movementSystem.update(registry, dt);
            collisionSystem.update(registry, &arena);

            bool playerDied = false;
            for (auto entity : registry.view<CollisionResultComponent>()) {
                auto& result = registry.getComponent<CollisionResultComponent>(entity);
                switch (result.result) {
                    case CollisionType::Food:
                        relocateFood(registry, food, gridWidth, gridHeight, &arena);
                        result.result = CollisionType::None;
                        break;
                    case CollisionType::Wall:
                    case CollisionType::Self:
                    case CollisionType::Snake:
                        if (entity == playerSnake) playerDied = true;
                        result.result = CollisionType::None;
                        break;
                    case CollisionType::None:
                        break;
                }
            }

            if (playerDied) {
                state = GameState::GameOver;
                menuSystem.setState(GameState::GameOver);
                animations.enableTunnelEffect(true, TunnelConfig::menu());
            }

            // Render ────────────────────────────────────────────────────────
            postFX.beginCapture();
            BeginDrawing();
            renderSystem.render(registry, renderMode, dt, &arena);
            EndDrawing();
            postFX.endCapture();

            BeginDrawing();
            postFX.applyAndPresent(dt);
            EndDrawing();
            continue;
        }

        // ── State: GameOver ───────────────────────────────────────────────────
        if (state == GameState::GameOver) {
            animations.update(dt);
            particles.update(dt);
            menuSystem.update(dt, particles);

            if (menuSystem.wantsMenu()) {
                menuSystem.consumeFlags();
                state = GameState::Menu;
                menuSystem.setState(GameState::Menu);
                resetGame(registry, inputSystem, playerSnake, aiSnake, food,
                          gridWidth, gridHeight, arena);
                animations.enableTunnelEffect(true, TunnelConfig::menu());
            }
            if (menuSystem.wantsQuit()) {
                menuSystem.consumeFlags();
                break;
            }

            // Render ────────────────────────────────────────────────────────
            BeginDrawing();
            ClearBackground(BLACK);
            particles.render();
            menuSystem.renderGameOver(textSystem, particles, &animations);
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}


