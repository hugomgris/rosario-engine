#pragma once
#include "ArenaGrid.hpp"

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