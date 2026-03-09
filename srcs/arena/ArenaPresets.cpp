#include "arena/ArenaPresets.hpp"
#include <cstdlib>
#include <algorithm>

// ─── InterLock1 ───────────────────────────────────────────────────────────────

void ArenaPresets::applyInterlock1(ArenaGrid& arena) {
    for (int i = 0; i < 15; ++i) {
        arena.setCell(i, 4,  CellType::SpawningSolid);
        arena.setCell(i, 5,  CellType::SpawningSolid);
        arena.setCell(i, 14, CellType::SpawningSolid);
        arena.setCell(i, 15, CellType::SpawningSolid);
        arena.setCell(i, 24, CellType::SpawningSolid);
        arena.setCell(i, 25, CellType::SpawningSolid);
    }

    int right = arena.getPlayWidth() - 1;
    for (int i = 0; i < 15; ++i) {
        arena.setCell(right - i, 9,  CellType::SpawningSolid);
        arena.setCell(right - i, 10, CellType::SpawningSolid);
        arena.setCell(right - i, 19, CellType::SpawningSolid);
        arena.setCell(right - i, 20, CellType::SpawningSolid);
    }
}

// ─── Spiral1 ──────────────────────────────────────────────────────────────────

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

// ─── Columns1 ─────────────────────────────────────────────────────────────────

void ArenaPresets::applyColumns1(ArenaGrid& arena) {
    int W = arena.getPlayWidth();
    int H = arena.getPlayHeight();

    for (int x = 3; x < W; x += 4)
        for (int y = 3; y < H; y += 4)
            arena.setCell(x, y, CellType::SpawningSolid);
}

// ─── Columns2 ─────────────────────────────────────────────────────────────────

void ArenaPresets::applyColumns2(ArenaGrid& arena) {
    int W = arena.getPlayWidth();
    int H = arena.getPlayHeight();

    for (int bx = 3; bx < W; bx += 7) {
        for (int by = 3; by < H; by += 7) {
            for (int dx = 0; dx < 4; ++dx)
                for (int dy = 0; dy < 4; ++dy)
                    arena.setCell(bx + dx, by + dy, CellType::SpawningSolid);
        }
    }
}

// ─── Cross ────────────────────────────────────────────────────────────────────

void ArenaPresets::applyCross(ArenaGrid& arena) {
    // Horizontal bar: row 14–16, columns 4–26 (gap at centre 13–17)
    for (int x = 4; x <= 26; ++x) {
        if (x >= 13 && x <= 17) continue;
        arena.setCell(x, 14, CellType::SpawningSolid);
        arena.setCell(x, 15, CellType::SpawningSolid);
        arena.setCell(x, 16, CellType::SpawningSolid);
    }
    // Vertical bar: col 14–16, rows 4–26 (gap at centre 13–17)
    for (int y = 4; y <= 26; ++y) {
        if (y >= 13 && y <= 17) continue;
        arena.setCell(14, y, CellType::SpawningSolid);
        arena.setCell(15, y, CellType::SpawningSolid);
        arena.setCell(16, y, CellType::SpawningSolid);
    }
}

// ─── Checkerboard ─────────────────────────────────────────────────────────────

void ArenaPresets::applyCheckerboard(ArenaGrid& arena) {
    for (int bx = 3; bx < 29; bx += 6) {
        for (int by = 3; by < 29; by += 6) {
            if (((bx / 6) + (by / 6)) % 2 == 0) {
                for (int dx = 0; dx < 3; ++dx)
                    for (int dy = 0; dy < 3; ++dy)
                        arena.setCell(bx + dx, by + dy, CellType::SpawningSolid);
            }
        }
    }
}

// ─── Maze ─────────────────────────────────────────────────────────────────────

void ArenaPresets::applyMaze(ArenaGrid& arena) {
    int rows[] = { 5, 10, 15, 20, 25 };
    for (int i = 0; i < 5; ++i) {
        int y = rows[i];
        if (i % 2 == 0) {
            for (int x = 0; x <= 24; ++x)
                arena.setCell(x, y, CellType::SpawningSolid);
        } else {
            for (int x = 6; x <= 30; ++x)
                arena.setCell(x, y, CellType::SpawningSolid);
        }
    }
}

// ─── Diamond ──────────────────────────────────────────────────────────────────

void ArenaPresets::applyDiamond(ArenaGrid& arena) {
    int cx = 15, cy = 15, r = 9;

    for (int dx = -r; dx <= r; ++dx) {
        int dy1 =  r - std::abs(dx);
        int dy2 = -(r - std::abs(dx));
        for (int dy : { dy1, dy2 }) {
            int x = cx + dx, y = cy + dy;
            if (x >= 0 && x < 31 && y >= 0 && y < 31)
                arena.setCell(x, y, CellType::SpawningSolid);
        }
    }
    // Thicken by one cell inward
    for (int dx = -(r-1); dx <= (r-1); ++dx) {
        int dy1 =  (r - 1) - std::abs(dx);
        int dy2 = -((r - 1) - std::abs(dx));
        for (int dy : { dy1, dy2 }) {
            int x = cx + dx, y = cy + dy;
            if (x >= 0 && x < 31 && y >= 0 && y < 31)
                arena.setCell(x, y, CellType::SpawningSolid);
        }
    }
}

// ─── Tunnels ──────────────────────────────────────────────────────────────────

void ArenaPresets::applyTunnels(ArenaGrid& arena) {
    int divRows[] = { 10, 20 };
    for (int row : divRows) {
        for (int x = 0; x < 31; ++x) {
            if (x <= 2 || (x >= 14 && x <= 16) || x >= 28) continue;
            arena.setCell(x, row,     CellType::SpawningSolid);
            arena.setCell(x, row + 1, CellType::SpawningSolid);
        }
    }
}

// ─── FourRooms ────────────────────────────────────────────────────────────────

void ArenaPresets::applyFourRooms(ArenaGrid& arena) {
    int cx = 15, cy = 15;

    for (int y = 3; y <= cy - 2; ++y)   arena.setCell(cx, y, CellType::SpawningSolid);
    for (int y = cy + 2; y <= 27; ++y)  arena.setCell(cx, y, CellType::SpawningSolid);
    for (int x = 3; x <= cx - 2; ++x)   arena.setCell(x, cy, CellType::SpawningSolid);
    for (int x = cx + 2; x <= 27; ++x)  arena.setCell(x, cy, CellType::SpawningSolid);
}

// ─── Random preset picker ─────────────────────────────────────────────────────

WallPreset ArenaPresets::getRandomPreset() {
    static const WallPreset presets[] = {
        WallPreset::InterLock1,
        WallPreset::Spiral1,
        WallPreset::Columns1,
        WallPreset::Columns2,
        WallPreset::Cross,
        WallPreset::Checkerboard,
        WallPreset::Maze,
        WallPreset::Diamond,
        WallPreset::Tunnels,
        WallPreset::FourRooms,
    };
    return presets[std::rand() % 10];
}
