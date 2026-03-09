#pragma once
#include "arena/ArenaGrid.hpp"

// ─── ArenaPresets ─────────────────────────────────────────────────────────────
//
// Static functions that fill an ArenaGrid with SpawningSolid cells according
// to named layout patterns. Cells are set to SpawningSolid rather than
// Obstacle so the spawn-animation timer can solidify them after a delay.
//
namespace ArenaPresets {
    void applyInterlock1  (ArenaGrid& arena);
    void applySpiral1     (ArenaGrid& arena);
    void applyColumns1    (ArenaGrid& arena);
    void applyColumns2    (ArenaGrid& arena);
    void applyCross       (ArenaGrid& arena);
    void applyCheckerboard(ArenaGrid& arena);
    void applyMaze        (ArenaGrid& arena);
    void applyDiamond     (ArenaGrid& arena);
    void applyTunnels     (ArenaGrid& arena);
    void applyFourRooms   (ArenaGrid& arena);

    WallPreset getRandomPreset();
}
