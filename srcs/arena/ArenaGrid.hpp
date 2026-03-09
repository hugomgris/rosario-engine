#pragma once
#include <vector>
#include <map>
#include <set>
#include <raylib.h>
#include "DataStructs.hpp"

// ─── Cell types ───────────────────────────────────────────────────────────────
//
// The arena grid stores typed cells. Walls are the permanent border.
// Obstacles are interior walls spawned from presets. SpawningSolid /
// DespawningSolid are transient states during the animation timers.
//
enum class CellType {
    Empty,
    Wall,
    Obstacle,
    Food,
    SpawningSolid,
    DespawningSolid
};

// ─── Preset identifiers ───────────────────────────────────────────────────────

enum class WallPreset {
    InterLock1,
    Spiral1,
    Columns1,
    Columns2,
    Cross,
    Checkerboard,
    Maze,
    Diamond,
    Tunnels,
    FourRooms,
};

// ─── ArenaGrid ────────────────────────────────────────────────────────────────
//
// Pure data class — no rendering, no ECS coupling.
//
// The grid is stored with a 1-cell-thick WALL border on all sides.
// Public API uses "game coordinates" (0 to width-1, 0 to height-1).
// Internal grid[y][x] uses the +1 offset automatically.
//
class ArenaGrid {
public:
    // width / height are the playable dimensions (border cells are added internally)
    ArenaGrid(int width, int height);

    // ── Game-coordinate cell access (0..width-1, 0..height-1) ────────────────
    void     setCell(int x, int y, CellType type);
    CellType getCell(int x, int y) const;
    bool     isWalkable(int x, int y) const;

    // Helper used by relocation: returns all empty playable cells
    std::vector<Vec2> getAvailableCells() const;

    // Obstacle placement ──────────────────────────────────────────────────────
    void spawnObstacle(int x, int y, int w, int h);

    // Grows a wall outward from an existing wall anchor in game coords.
    // width/height may be negative (growth direction).
    void growWall(int x, int y, int w, int h);

    void clearCell(int x, int y);
    void clearArena();

    // Preset application (called by ArenaPresets namespace)
    void transformArenaWithPreset(WallPreset preset);

    // Outline extraction (used by AnimationSystem for tunnel-line geometry)
    // offsetX/offsetY are the screen-pixel offset of cell (0,0).
    std::vector<std::vector<Vector2>> getAllOutlines(int offsetX, int offsetY) const;

    // Full grid access (needed by presets that read dimensions)
    const std::vector<std::vector<CellType>>& getGrid() const { return grid; }

    // Playable dimensions
    int getPlayWidth()  const { return gridWidth  - 2; }
    int getPlayHeight() const { return gridHeight - 2; }

    // Internal full dimensions (including border)
    int getFullWidth()  const { return gridWidth;  }
    int getFullHeight() const { return gridHeight; }

    // ── Spawn / despawn animation timers ─────────────────────────────────────
    // "Spawn" = preset obstacles fade in after a delay.
    // "Despawn" = obstacles fade out and are then cleared.

    void  beginSpawn(float solidifyDelay);
    void  tickSpawnTimer(float dt);
    bool  isSpawning()         const;
    float getSpawnFadeProgress() const;   // 0→1 as fade-in completes

    void  beginDespawn(float delay);
    void  tickDespawnTimer(float dt);
    bool  isDespawning()       const;
    float getDespawnFadeProgress() const; // 1→0 as fade-out completes

private:
    std::vector<std::vector<CellType>> grid;
    int gridWidth;   // playWidth  + 2
    int gridHeight;  // playHeight + 2

    // Spawn animation
    float spawnTimer    = 0.0f;
    float spawnDuration = 0.0f;
    float fadeTimer     = 0.0f;
    float fadeDuration  = 0.1f;

    // Despawn animation
    float despawnTimer    = 0.0f;
    float fadeOutTimer    = 0.0f;
    float fadeOutDuration = 0.15f;
};
