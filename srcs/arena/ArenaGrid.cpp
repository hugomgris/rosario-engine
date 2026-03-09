#include "arena/ArenaGrid.hpp"
#include "arena/ArenaPresets.hpp"
#include <map>
#include <set>
#include <cmath>
#include <iostream>

// ─── Construction ─────────────────────────────────────────────────────────────

ArenaGrid::ArenaGrid(int width, int height)
    : gridWidth(width + 2), gridHeight(height + 2) {
    clearArena();
}

// ─── Cell access (game coordinates, +1 internal offset) ──────────────────────

void ArenaGrid::setCell(int x, int y, CellType type) {
    int gx = x + 1;
    int gy = y + 1;
    if (gy < 0 || gy >= gridHeight || gx < 0 || gx >= gridWidth) return;
    grid[gy][gx] = type;
}

CellType ArenaGrid::getCell(int x, int y) const {
    if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2)
        return CellType::Wall;
    return grid[y + 1][x + 1];
}

bool ArenaGrid::isWalkable(int x, int y) const {
    if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2)
        return false;
    return grid[y + 1][x + 1] == CellType::Empty;
}

std::vector<Vec2> ArenaGrid::getAvailableCells() const {
    std::vector<Vec2> free;
    for (int y = 1; y < gridHeight - 1; ++y)
        for (int x = 1; x < gridWidth - 1; ++x)
            if (grid[y][x] == CellType::Empty)
                free.push_back({ x - 1, y - 1 });
    return free;
}

// ─── Obstacle placement ───────────────────────────────────────────────────────

void ArenaGrid::spawnObstacle(int x, int y, int w, int h) {
    for (int i = 0; i < w; ++i)
        for (int j = 0; j < h; ++j)
            setCell(x + i, y + j, CellType::Obstacle);
}

void ArenaGrid::growWall(int x, int y, int w, int h) {
    if (x < -1 || x >= gridWidth - 1 || y < -1 || y >= gridHeight - 1) {
        std::cerr << "growWall: anchor out of bounds\n";
        return;
    }
    if (getCell(x, y) != CellType::Wall) {
        std::cerr << "growWall: base node is not a wall\n";
        return;
    }

    int xStep = (w >= 0) ? 1 : -1;
    int yStep = (h >= 0) ? 1 : -1;

    for (int i = x; i != x + w + xStep; i += xStep) {
        if (i < -1 || i >= gridWidth - 1) continue;
        for (int j = y; j != y + h + yStep; j += yStep) {
            if (j < -1 || j >= gridHeight - 1) continue;
            setCell(i, j, CellType::Wall);
        }
    }
}

void ArenaGrid::clearCell(int x, int y) {
    setCell(x, y, CellType::Empty);
}

void ArenaGrid::clearArena() {
    grid.assign(gridHeight, std::vector<CellType>(gridWidth, CellType::Empty));
    for (int y = 0; y < gridHeight; ++y)
        for (int x = 0; x < gridWidth; ++x)
            if (x == 0 || x == gridWidth - 1 || y == 0 || y == gridHeight - 1)
                grid[y][x] = CellType::Wall;
}

// ─── Preset dispatch ──────────────────────────────────────────────────────────

void ArenaGrid::transformArenaWithPreset(WallPreset preset) {
    clearArena();
    switch (preset) {
        case WallPreset::InterLock1:  ArenaPresets::applyInterlock1(*this);  break;
        case WallPreset::Spiral1:     ArenaPresets::applySpiral1(*this);     break;
        case WallPreset::Columns1:    ArenaPresets::applyColumns1(*this);    break;
        case WallPreset::Columns2:    ArenaPresets::applyColumns2(*this);    break;
        case WallPreset::Cross:       ArenaPresets::applyCross(*this);       break;
        case WallPreset::Checkerboard:ArenaPresets::applyCheckerboard(*this);break;
        case WallPreset::Maze:        ArenaPresets::applyMaze(*this);        break;
        case WallPreset::Diamond:     ArenaPresets::applyDiamond(*this);     break;
        case WallPreset::Tunnels:     ArenaPresets::applyTunnels(*this);     break;
        case WallPreset::FourRooms:   ArenaPresets::applyFourRooms(*this);   break;
    }
}

// ─── Outline extraction ───────────────────────────────────────────────────────
//
// The algorithm traces polygon outlines of all connected wall regions.
// It's a half-edge approach: for each non-wall cell, emit directed edges
// along its sides that border a wall cell. Edges that would be shared by
// two non-wall cells cancel each other, leaving only the outer boundary.
//
// The resulting outlines are in screen-pixel coordinates, offset by
// (offsetX, offsetY) and scaled by squareSize == 1 (the caller scales).
//
// This geometry is what the AnimationSystem uses for the tunnel-line effect.

std::vector<std::vector<Vector2>> ArenaGrid::getAllOutlines(int offsetX, int offsetY) const {
    struct IVec2 {
        int x, y;
        bool operator<(const IVec2& o) const {
            return x < o.x || (x == o.x && y < o.y);
        }
        bool operator==(const IVec2& o) const {
            return x == o.x && y == o.y;
        }
    };

    auto isWall = [&](int c, int r) -> bool {
        if (r < 0 || r >= gridHeight || c < 0 || c >= gridWidth) return true;
        return grid[r][c] != CellType::Empty &&
               grid[r][c] != CellType::Food;
    };

    std::map<IVec2, IVec2> next;
    std::set<IVec2> cancelled;

    auto addEdge = [&](IVec2 a, IVec2 b) {
        if (cancelled.count(a)) return;
        if (next.count(a)) {
            next.erase(a);
            cancelled.insert(a);
        } else {
            next[a] = b;
        }
    };

    // Emit edges around each non-wall cell in full-grid coords.
    for (int r = 0; r < gridHeight; ++r) {
        for (int c = 0; c < gridWidth; ++c) {
            if (isWall(c, r)) continue;
            if (isWall(c,   r-1)) addEdge({c,   r  }, {c+1, r  });
            if (isWall(c+1, r  )) addEdge({c+1, r  }, {c+1, r+1});
            if (isWall(c,   r+1)) addEdge({c+1, r+1}, {c,   r+1});
            if (isWall(c-1, r  )) addEdge({c,   r+1}, {c,   r  });
        }
    }

    std::vector<std::vector<Vector2>> allOutlines;

    while (!next.empty()) {
        // Pick the lexicographically smallest node as loop start
        IVec2 start = next.begin()->first;
        for (const auto& kv : next)
            if (kv.first < start) start = kv.first;

        std::vector<Vector2> outline;
        std::vector<IVec2> loopNodes;
        IVec2 prev = start;
        IVec2 cur  = next[start];
        loopNodes.push_back(start);
        int limit = (int)next.size() + 2;

        while (!(cur == start) && --limit > 0) {
            IVec2 nxt = next[cur];
            int dx1 = cur.x - prev.x, dy1 = cur.y - prev.y;
            int dx2 = nxt.x - cur.x,  dy2 = nxt.y - cur.y;
            if (dx1 != dx2 || dy1 != dy2) {
                outline.push_back({
                    static_cast<float>(offsetX + cur.x),
                    static_cast<float>(offsetY + cur.y)
                });
            }
            loopNodes.push_back(cur);
            prev = cur;
            cur  = nxt;
        }

        // Check the corner at start
        if (!outline.empty()) {
            int dx1 = cur.x - prev.x,  dy1 = cur.y - prev.y;
            int dx2 = start.x - cur.x, dy2 = start.y - cur.y;
            if (dx1 != dx2 || dy1 != dy2)
                outline.push_back({
                    static_cast<float>(offsetX + start.x),
                    static_cast<float>(offsetY + start.y)
                });
        }

        if (!outline.empty())
            allOutlines.push_back(outline);

        for (const IVec2& node : loopNodes)
            next.erase(node);
    }

    return allOutlines;
}

// ─── Spawn timer ─────────────────────────────────────────────────────────────

void ArenaGrid::beginSpawn(float solidifyDelay) {
    spawnTimer    = solidifyDelay;
    spawnDuration = solidifyDelay;
}

void ArenaGrid::tickSpawnTimer(float dt) {
    if (spawnTimer > 0.0f) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f) {
            for (int y = 1; y < gridHeight - 1; ++y)
                for (int x = 1; x < gridWidth - 1; ++x)
                    if (grid[y][x] == CellType::SpawningSolid)
                        grid[y][x] = CellType::Obstacle;
            fadeTimer = fadeDuration;
        }
        return;
    }
    if (fadeTimer > 0.0f)
        fadeTimer -= dt;
}

bool ArenaGrid::isSpawning() const {
    return spawnTimer > 0.0f || fadeTimer > 0.0f;
}

float ArenaGrid::getSpawnFadeProgress() const {
    if (fadeDuration <= 0.0f || fadeTimer <= 0.0f) return 1.0f;
    return 1.0f - (fadeTimer / fadeDuration);
}

// ─── Despawn timer ────────────────────────────────────────────────────────────

void ArenaGrid::beginDespawn(float delay) {
    for (int y = 1; y < gridHeight - 1; ++y)
        for (int x = 1; x < gridWidth - 1; ++x)
            if (grid[y][x] == CellType::Obstacle)
                grid[y][x] = CellType::DespawningSolid;
    despawnTimer = delay;
}

void ArenaGrid::tickDespawnTimer(float dt) {
    if (despawnTimer > 0.0f) {
        despawnTimer -= dt;
        if (despawnTimer <= 0.0f)
            fadeOutTimer = fadeOutDuration;
        return;
    }
    if (fadeOutTimer > 0.0f) {
        fadeOutTimer -= dt;
        if (fadeOutTimer <= 0.0f) {
            for (int y = 1; y < gridHeight - 1; ++y)
                for (int x = 1; x < gridWidth - 1; ++x)
                    if (grid[y][x] == CellType::DespawningSolid)
                        grid[y][x] = CellType::Empty;
        }
    }
}

bool ArenaGrid::isDespawning() const {
    return despawnTimer > 0.0f || fadeOutTimer > 0.0f;
}

float ArenaGrid::getDespawnFadeProgress() const {
    if (fadeOutDuration <= 0.0f || fadeOutTimer <= 0.0f) return 0.0f;
    return fadeOutTimer / fadeOutDuration;
}
