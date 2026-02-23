#include "../../incs/ArenaPresets.hpp"
#include <cstdlib>

void ArenaPresets::applyInterlock1(Arena& arena) {
	for (int i = 0; i < 15; i++) {
		arena.setCell(i, 4, CellType::SpawningSolid);
		arena.setCell(i, 5, CellType::SpawningSolid);

		arena.setCell(i, 14, CellType::SpawningSolid);
		arena.setCell(i, 15, CellType::SpawningSolid);

		arena.setCell(i, 24, CellType::SpawningSolid);
		arena.setCell(i, 25, CellType::SpawningSolid);
	}

	int right = arena.getGrid().size() - 3;
	for (int i = 0; i < 15; i++) {
		arena.setCell(right - i, 9, CellType::SpawningSolid);
		arena.setCell(right - i, 10, CellType::SpawningSolid);

		arena.setCell(right - i, 19, CellType::SpawningSolid);
		arena.setCell(right - i, 20, CellType::SpawningSolid);
	}
}

void ArenaPresets::applySpiral1(Arena& arena) {
	const int gap = 3;
	auto grid = arena.getGrid();

	int W = (int)grid[0].size() - 2;
	int H = (int)grid.size()    - 2;

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

void ArenaPresets::applyColumns1(Arena& arena) {
	auto grid = arena.getGrid();

	int W = (int)grid[0].size() - 2;
	int H = (int)grid.size()    - 2;

	for (int x = 3; x < W; x += 4) {
		for (int y = 3; y < H; y += 4) {
			arena.setCell(x, y, CellType::SpawningSolid);
		}
	}
}

void ArenaPresets::applyColumns2(Arena& arena) {
	auto grid = arena.getGrid();

	int W = (int)grid[0].size() - 2;
	int H = (int)grid.size()    - 2;

	for (int x = 3; x < W; x += 7) {
		for (int y = 3; y < H; y += 7) {
			arena.setCell(x,     y,     CellType::SpawningSolid);
			arena.setCell(x + 1, y,     CellType::SpawningSolid);
			arena.setCell(x + 2, y,     CellType::SpawningSolid);
			arena.setCell(x + 3, y,     CellType::SpawningSolid);

			arena.setCell(x,     y + 1, CellType::SpawningSolid);
			arena.setCell(x + 1, y + 1, CellType::SpawningSolid);
			arena.setCell(x + 2, y + 1, CellType::SpawningSolid);
			arena.setCell(x + 3, y + 1, CellType::SpawningSolid);

			arena.setCell(x,     y + 2, CellType::SpawningSolid);
			arena.setCell(x + 1, y + 2, CellType::SpawningSolid);
			arena.setCell(x + 2, y + 2, CellType::SpawningSolid);
			arena.setCell(x + 3, y + 2, CellType::SpawningSolid);

			arena.setCell(x,     y + 3, CellType::SpawningSolid);
			arena.setCell(x + 1, y + 3, CellType::SpawningSolid);
			arena.setCell(x + 2, y + 3, CellType::SpawningSolid);
			arena.setCell(x + 3, y + 3, CellType::SpawningSolid);
		}
	}
}


void ArenaPresets::applyCross(Arena& arena) {
	// Horizontal bar: row 14–16, columns 4–26 (skipping dead center 13–17)
	for (int x = 4; x <= 26; x++) {
		if (x >= 13 && x <= 17) continue;	// gap at center
		arena.setCell(x, 14, CellType::SpawningSolid);
		arena.setCell(x, 15, CellType::SpawningSolid);
		arena.setCell(x, 16, CellType::SpawningSolid);
	}

	// Vertical bar: col 14–16, rows 4–26 (skipping dead center)
	for (int y = 4; y <= 26; y++) {
		if (y >= 13 && y <= 17) continue;	// gap at center
		arena.setCell(14, y, CellType::SpawningSolid);
		arena.setCell(15, y, CellType::SpawningSolid);
		arena.setCell(16, y, CellType::SpawningSolid);
	}
}

void ArenaPresets::applyCheckerboard(Arena& arena) {
	// Step 6 gives a 3-cell block + 3-cell gap rhythm.
	// Starting at x=3, y=3 keeps the first block away from the border walls.
	for (int bx = 3; bx < 29; bx += 6) {
		for (int by = 3; by < 29; by += 6) {
			// Alternate which positions get blocks
			if (((bx / 6) + (by / 6)) % 2 == 0) {
				for (int dx = 0; dx < 3; dx++)
					for (int dy = 0; dy < 3; dy++)
						arena.setCell(bx + dx, by + dy, CellType::SpawningSolid);
			}
		}
	}
}

void ArenaPresets::applyMaze(Arena& arena) {
	// Rows at y = 5, 10, 15, 20, 25
	// Even index → comb from left, gap on right
	// Odd index  → comb from right, gap on left
	int rows[]  = { 5, 10, 15, 20, 25 };
	int nRows   = 5;

	for (int i = 0; i < nRows; i++) {
		int y = rows[i];
		if (i % 2 == 0) {
			// Left comb: x = 0..24, gap at 25..30
			for (int x = 0; x <= 24; x++)
				arena.setCell(x, y, CellType::SpawningSolid);
		} else {
			// Right comb: x = 6..30, gap at 0..5
			for (int x = 6; x <= 30; x++)
				arena.setCell(x, y, CellType::SpawningSolid);
		}
	}
}

void ArenaPresets::applyDiamond(Arena& arena) {
	int cx = 15, cy = 15;
	int r  = 9;		// half-size of the diamond

	// Walk the perimeter of a diamond (|dx| + |dy| == r)
	for (int dx = -r; dx <= r; dx++) {
		int dy1 =  r - abs(dx);
		int dy2 = -(r - abs(dx));

		int x = cx + dx;

		// top and bottom edges of the diamond
		for (int dy : {dy1, dy2}) {
			int y = cy + dy;
			if (x >= 0 && x < 31 && y >= 0 && y < 31)
				arena.setCell(x, y, CellType::SpawningSolid);
		}
	}

	// Thicken by one cell inward so it's a solid ring, not just an outline
	for (int dx = -(r-1); dx <= (r-1); dx++) {
		int dy1 =  (r - 1) - abs(dx);
		int dy2 = -((r - 1) - abs(dx));

		int x = cx + dx;
		for (int dy : {dy1, dy2}) {
			int y = cy + dy;
			if (x >= 0 && x < 31 && y >= 0 && y < 31)
				arena.setCell(x, y, CellType::SpawningSolid);
		}
	}
}

void ArenaPresets::applyTunnels(Arena& arena) {
	// Two dividing rows
	int divRows[] = { 10, 20 };

	for (int row : divRows) {
		for (int x = 0; x < 31; x++) {
			// Three openings: left edge, center, right edge
			if (x <= 2 || (x >= 14 && x <= 16) || x >= 28)
				continue;
			arena.setCell(x, row,     CellType::SpawningSolid);
			arena.setCell(x, row + 1, CellType::SpawningSolid);
		}
	}
}

void ArenaPresets::applyFourRooms(Arena& arena) {
	int cx = 15, cy = 15;

	// Top wall: column cx, rows 3..(cy-2)
	for (int y = 3; y <= cy - 2; y++)
		arena.setCell(cx, y, CellType::SpawningSolid);

	// Bottom wall: column cx, rows (cy+2)..27
	for (int y = cy + 2; y <= 27; y++)
		arena.setCell(cx, y, CellType::SpawningSolid);

	// Left wall: row cy, columns 3..(cx-2)
	for (int x = 3; x <= cx - 2; x++)
		arena.setCell(x, cy, CellType::SpawningSolid);

	// Right wall: row cy, columns (cx+2)..27
	for (int x = cx + 2; x <= 27; x++)
		arena.setCell(x, cy, CellType::SpawningSolid);
}

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

	int count = static_cast<int>(sizeof(presets) / sizeof(presets[0]));
	return presets[std::rand() % count];
}