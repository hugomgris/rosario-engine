#include "../../incs/Arena.hpp"
#include "../../incs/ArenaPresets.hpp"

// NOTE:
// grid is stored with a 1-cell thick WALL border on all sides.
// Game coordinates are translated via +1 offset.
// Direct grid[y][x] access must only be used for full-grid operations.

Arena::Arena(int width, int height, int squareSize):
		gridWidth(width + 2), gridHeight(height + 2), squareSize(squareSize), foodPosition(Vector2{-1,-1}) {
	clearArena();
}

// grid manipulation
std::vector<std::vector<CellType>> Arena::getGrid() const { return grid; }

void Arena::setCell(int x, int y, CellType type) {
	int gx = x + 1;
	int gy = y + 1;

	if (gy < 0 || gy >= gridHeight ||
		gx < 0 || gx >= gridWidth)
		return;

	grid[gy][gx] = type;
}

CellType Arena::getCell(int x, int y) const { 
	// Bounds check for game coordinates
	if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2) {
		return CellType::Wall;	// Out of bounds treated as wall
	}
	return grid[y + 1][x + 1]; 
}

bool Arena::isWalkable(int x, int y) const {
	// Bounds check
	if (x < 0 || x >= gridWidth - 2 || y < 0 || y >= gridHeight - 2) {
		return false;
	}
	
	CellType target = grid[y + 1][x + 1];

	if (target != CellType::Empty)
		return false;

	return true;
}

void Arena::setFoodCell(int x, int y) { 
	foodPosition.x = x;
	foodPosition.y = y;
	setCell(x, y, CellType::Food);

	//DEBUG output
	/* std::cout << "Food set at (" << x << ", " << y << ")" << std::endl;

	int foodAmount = 0;
	for (int i = 0; i < gridHeight; i++) {
		for (int j = 0; j < gridWidth; j++) {
			if (grid[i][j] == CellType::Food) {
				foodAmount++;
				std::cout << "Food found at grid[" << i << "][" << j << "]" << std::endl;
			}
		}
	}

	std::cout << "Total food in arena: " << foodAmount << std::endl; */
}

Vector2 Arena::getFoodPosition() const {
	return foodPosition;
}

const std::vector<Vec2> Arena::getAvailableCells() const {
	std::vector<Vec2> freePos;
	for (int y = 1; y < gridHeight - 1; y++) {
		for (int x = 1; x < gridWidth - 1; x++) {
			if (grid[y][x] == CellType::Empty) {
				freePos.push_back(Vec2{x - 1, y - 1});
			}
		}
	}
	return freePos;
}

//obstacle management
void Arena::spawnObstacle(int x, int y, int width, int height) {
	// Translate to arena grid coordinates and use setCell for consistency
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			setCell(x + i, y + j, CellType::Obstacle);
		}
	}
}

// works by: targeting a WALL TYPE node, checking the direction of growth via width/height sign
void Arena::growWall(int x, int y, int width, int height) {
	// Check if starting position is valid in game coordinates
	if (x < -1 || x >= gridWidth - 1 || y < -1 || y >= gridHeight - 1) {
		std::cout << "error: wall growth: anchor out of bounds!" << std::endl;
		return;
	}

	if (getCell(x, y) != CellType::Wall) {
		std::cout << "error: wall growth: base node was not a wall!" << std::endl;
		return;
	}

	int xStep = (width >= 0) ? 1 : -1;
	int yStep = (height >= 0) ? 1 : -1;

	// Grow from the anchor position
	for (int i = x; i != x + width + xStep; i += xStep) {
		if (i < -1 || i >= gridWidth - 1) continue;
		for (int j = y; j != y + height + yStep; j += yStep) {
			if (j < -1 || j >= gridHeight - 1) continue;
			setCell(i, j, CellType::Wall);
		}
	}
}

void Arena::clearCell(int x, int y) {
	setCell(x, y, CellType::Empty);
}

void Arena::clearArena() {
	// Clear everything
	grid.assign(gridHeight, std::vector<CellType>(gridWidth, CellType::Empty));

	// Border walls (still direct access, but guaranteed safe)
	for (int y = 0; y < gridHeight; y++) {
		for (int x = 0; x < gridWidth; x++) {
			if (x == 0 || x == gridWidth - 1 ||
				y == 0 || y == gridHeight - 1)
			{
				grid[y][x] = CellType::Wall;
			}
		}
	}
}

void Arena::render(const Renderer& renderer) const {
    bool spawning = (fadeTimer > 0.0f);
    bool despawning = (fadeOutTimer > 0.0f);

    float spawnEased = 0.0f;
    if (spawning) {
        float p = getSpawnFadeProgress();
        spawnEased = p * p;
    }

    float despawnEased = 0.0f;
    if (despawning) {
        float p = getDespawnFadeProgress();
        despawnEased = p * p;
    }

    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            if (grid[y][x] == CellType::Empty) continue;
            if (grid[y][x] == CellType::SpawningSolid) continue;

            if (grid[y][x] == CellType::Wall) {
                renderer.drawBorderBrick(x, y, wallColor);
            }
            else if (grid[y][x] == CellType::Obstacle) {
                if (spawning) {
                    Color faded = wallColor;
                    faded.a = static_cast<unsigned char>(spawnEased * 255.0f);
                    renderer.drawBorderBrick(x, y, faded);
                } else {
                    renderer.drawBorderBrick(x, y, wallColor);
                }
            }
            else if (grid[y][x] == CellType::DespawningSolid) {
                if (despawning) {
                    Color faded = wallColor;
                    faded.a = static_cast<unsigned char>(despawnEased * 255.0f);
                    renderer.drawBorderBrick(x, y, faded);
                } else {
                    // In the countdown phase (waiting for last line) — full opacity
                    renderer.drawBorderBrick(x, y, wallColor);
                }
            }
        }
    }
}

std::vector<std::vector<Vector2>> Arena::getAllOutlines(int offsetX, int offsetY) {
	struct IVec2 {
		int x, y;
		bool operator<(const IVec2& o) const { return x < o.x || (x == o.x && y < o.y); }
		bool operator==(const IVec2& o) const { return x == o.x && y == o.y; }
	};

	auto isWall = [&](int c, int r) -> bool {
		if (r < 0 || r >= gridHeight || c < 0 || c >= gridWidth) return true;
		return grid[r][c] != CellType::Empty &&
			grid[r][c] != CellType::Food  &&
			grid[r][c] != CellType::Snake_A &&
			grid[r][c] != CellType::Snake_B;
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

	for (int r = 0; r < gridHeight; r++) {
		for (int c = 0; c < gridWidth; c++) {
			if (isWall(c, r)) continue;
			if (isWall(c,   r-1)) addEdge({c,   r  }, {c+1, r  });
			if (isWall(c+1, r  )) addEdge({c+1, r  }, {c+1, r+1});
			if (isWall(c,   r+1)) addEdge({c+1, r+1}, {c,   r+1});
			if (isWall(c-1, r  )) addEdge({c,   r+1}, {c,   r  });
		}
	}

	std::vector<std::vector<Vector2>> allOutlines;

	// Keep extracting loops until the edge map is empty
	while (!next.empty()) {
		IVec2 start = next.begin()->first;
		for (auto& kv : next)
			if (kv.first < start) start = kv.first;

		// walk once to collect corners
		std::vector<Vector2> outline;
		std::vector<IVec2> loopNodes;	// track every node in this loop
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
					static_cast<float>(offsetX + cur.x * squareSize),
					static_cast<float>(offsetY + cur.y * squareSize)
				});
			}
			loopNodes.push_back(cur);
			prev = cur;
			cur  = nxt;
		}
		// Check start corner
		if (!outline.empty()) {
			int dx1 = cur.x - prev.x,   dy1 = cur.y - prev.y;
			int dx2 = start.x - cur.x,  dy2 = start.y - cur.y;
			if (dx1 != dx2 || dy1 != dy2)
				outline.push_back({
					static_cast<float>(offsetX + start.x * squareSize),
					static_cast<float>(offsetY + start.y * squareSize)
				});
		}

		if (!outline.empty())
			allOutlines.push_back(outline);

		// erase every node that belonged to this loop
		for (const IVec2& node : loopNodes)
			next.erase(node);
	}

	return allOutlines;
}

void Arena::transformArenaWithPreset(WallPreset preset) {
	clearArena();

	switch (preset) {
		case WallPreset::InterLock1:
			ArenaPresets::applyInterlock1(*this);
			break;
		case WallPreset::Spiral1:
			ArenaPresets::applySpiral1(*this);
			break;
		case WallPreset::Columns1:
			ArenaPresets::applyColumns1(*this);
			break;
		case WallPreset::Columns2:
			ArenaPresets::applyColumns2(*this);
			break;
		case WallPreset::Cross:
			ArenaPresets::applyCross(*this);
			break;
		case WallPreset::Checkerboard:
			ArenaPresets::applyCheckerboard(*this);
			break;
		case WallPreset::Maze:
			ArenaPresets::applyMaze(*this);
			break;
		case WallPreset::Diamond:
			ArenaPresets::applyDiamond(*this);
			break;
		case WallPreset::Tunnels:
			ArenaPresets::applyTunnels(*this);
			break;
		case WallPreset::FourRooms:
			ArenaPresets::applyFourRooms(*this);
			break;
		default:
			std::cout << "Unknown wall preset!" << std::endl;
	}
}

// spawn management
bool Arena::isSpawning() const { return spawnTimer > 0.0f || fadeTimer > 0.0f; }

void Arena::tickSpawnTimer(float deltaTime) {
	// Phase 1: countdown to solidification
	if (spawnTimer > 0.0f) {
		spawnTimer -= deltaTime;

		if (spawnTimer <= 0.0f) {
			// Solidify cells
			for (int y = 1; y < gridHeight - 1; y++)
				for (int x = 1; x < gridWidth - 1; x++)
					if (grid[y][x] == CellType::SpawningSolid)
						grid[y][x] = CellType::Obstacle;

			// launch the fade-in timer
			fadeTimer = fadeDuration;
		}
		return;
	}

	// Phase 2: fade-in after solidification
	if (fadeTimer > 0.0f)
		fadeTimer -= deltaTime;
}

void Arena::beginSpawn(float solidifyDelay) {
	spawnTimer = solidifyDelay;
	spawnDuration = solidifyDelay;
};

float Arena::getSpawnFadeProgress() const {
	if (fadeDuration <= 0.0f) return 1.0f;
	if (fadeTimer <= 0.0f)    return 1.0f;	// fully opaque once done

	return 1.0f - (fadeTimer / fadeDuration);
}

// despawn management
void Arena::beginDespawn(float delay) {
	for (int y = 1; y < gridHeight - 1; y++)
		for (int x = 1; x < gridWidth - 1; x++)
			if (grid[y][x] == CellType::Obstacle)
				grid[y][x] = CellType::DespawningSolid;

	despawnTimer = delay;
}

void Arena::tickDespawnTimer(float deltaTime) {
	if (despawnTimer > 0.0f) {
		despawnTimer -= deltaTime;
		if (despawnTimer <= 0.0f)
			fadeOutTimer = fadeOutDuration;
		return;
	}

	if (fadeOutTimer > 0.0f) {
		fadeOutTimer -= deltaTime;
		if (fadeOutTimer <= 0.0f) {
			// Fade complete — clear the cells
			for (int y = 1; y < gridHeight - 1; y++)
				for (int x = 1; x < gridWidth - 1; x++)
					if (grid[y][x] == CellType::DespawningSolid)
						grid[y][x] = CellType::Empty;

			// TODO
			for (int y = 0; y < gridHeight; y++)
				for (int x = 0; x < gridWidth; x++)
					if (x == 0 || x == gridWidth - 1 || y == 0 || y == gridHeight - 1)
						grid[y][x] = CellType::Wall;
		}
	}
}

float Arena::getDespawnFadeProgress() const {
	if (fadeOutDuration <= 0.0f) return 0.0f;
	if (fadeOutTimer <= 0.0f)    return 0.0f;
	return fadeOutTimer / fadeOutDuration;
}

bool Arena::isDespawning() const {
	return despawnTimer > 0.0f || fadeOutTimer > 0.0f;
}

void Arena::startFadeOut() {
	fadeOutTimer = fadeOutDuration;
}