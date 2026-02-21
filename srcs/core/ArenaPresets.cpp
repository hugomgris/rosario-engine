#include "../../incs/ArenaPresets.hpp"

void ArenaPresets::applyInterlock1(Arena& arena) {
	for (int i = 0; i < 15; i++) {
		arena.setCell(i, 4, CellType::Wall);
		arena.setCell(i, 5, CellType::Wall);

		arena.setCell(i, 14, CellType::Wall);
		arena.setCell(i, 15, CellType::Wall);

		arena.setCell(i, 24, CellType::Wall);
		arena.setCell(i, 25, CellType::Wall);
	}

	// Right-side horizontal walls
	int right = arena.getGrid().size() - 3;
	for (int i = 0; i < 15; i++) {
		arena.setCell(right - i, 9, CellType::Wall);
		arena.setCell(right - i, 10, CellType::Wall);

		arena.setCell(right - i, 19, CellType::Wall);
		arena.setCell(right - i, 20, CellType::Wall);
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
			arena.setCell(xx, yy, CellType::Wall);
	};
	auto vline = [&](int xx, int y0, int y1) {
		if (y0 > y1) std::swap(y0, y1);
		for (int yy = y0; yy <= y1; ++yy)
			arena.setCell(xx, yy, CellType::Wall);
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