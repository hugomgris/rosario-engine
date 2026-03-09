#pragma once
#include "Arena.hpp"

namespace ArenaPresets {
    void applyInterlock1(Arena& arena);
    void applySpiral1(Arena& arena);
    void applyColumns1(Arena& arena);
    void applyColumns2(Arena& arena);
    void applyCross(Arena& arena);
    void applyCheckerboard(Arena& arena);
    void applyMaze(Arena& arena);
    void applyDiamond(Arena& arena);
    void applyTunnels(Arena& arena);
    void applyFourRooms(Arena& arena);

    // Random picker
    WallPreset getRandomPreset();
}