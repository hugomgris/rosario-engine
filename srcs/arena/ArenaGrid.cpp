#include "ArenaGrid.hpp"

// construction
ArenaGrid::ArenaGrid(int width, int height)
	: _gridWidth(width + 2), _gridHeight(height + 2),
	  _defaultGridWidth(width + 2), _defaultGridHeight(height + 2) {
	clearArena();
}

// cell access
void ArenaGrid::setCell(int x, int y, CellType type) {
	int gx = x + 1;
	int gy = y + 1;
	if (gy <  0 || gy >= _gridHeight || gx < 0 || gx >= _gridWidth) return;
	_grid[gy][gx] = type;
}

CellType ArenaGrid::getCell(int x, int y) const {
	if (y < 0 || y >= _gridHeight - 2 || x < 0 || x >= _gridWidth - 2) {
		return CellType::Wall;
	}
	return _grid[y + 1][x + 1];
}

bool ArenaGrid::isWalkable(int x, int y) const {
	if (y < 0 || y >= _gridHeight - 2 || x < 0 || x >= _gridWidth - 2) {
		return false;
	}
	return _grid[y + 1][x + 1] == CellType::Empty;
};

std::vector<Vec2> ArenaGrid::getAvailableCells() const {
	std::vector<Vec2> free;

	for (int y = 1; y < _gridHeight - 1; ++y) {
		for (int x = 1; x < _gridWidth - 1; ++x) {
			if (_grid[y][x] == CellType::Empty) {
				free.push_back({ x - 1, y - 1 });
			}
		}
	}
	return free;
}

std::vector<std::vector<CellType>> ArenaGrid::getGrid() const { return _grid; }

// obstacle placement
void ArenaGrid::spawnObstacle(int x, int y, int w, int h) {
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
			setCell(x + i, y + j, CellType::Obstacle);
		}
	}
}

void ArenaGrid::growWall(int x, int y, int w, int h) {
	if (y < -1 || y > _gridHeight - 1 || x < -1 || x >= _gridWidth - 1) {
		std::cerr << "growWall: anchor out of bounds" << std::endl;
		return;
	}

	if (getCell(x, y) != CellType::Wall) {
		std::cerr << "growWall: base node is not a wall" << std::endl;
		return;
	}

	int xStep = (w >= 0) ? 1 : -1;
	int yStep = (h >= 0) ? 1 : -1;

	for (int i = x; i != x + w + xStep; i += xStep) {
		if (i < -1 || i >= _gridWidth - 1) continue;
		for (int j = y; j != y + h + yStep; j += yStep) {
			if (j < -1 || j >= _gridHeight - 1) continue;
			
			setCell(i, j ,CellType::Wall);
		}
	}
}

void ArenaGrid::clearCell(int x, int y) {
	setCell(x, y, CellType::Empty);
}

void ArenaGrid::clearArena() {
	_grid.assign(_gridHeight, std::vector<CellType>(_gridWidth, CellType::Empty));
	for (int y = 0; y < _gridHeight; ++y)
		for (int x = 0; x < _gridWidth; ++x)
			if (x == 0 || x == _gridWidth - 1 || y == 0 || y == _gridHeight - 1)
				_grid[y][x] = CellType::Wall;
}

// preset dispatchng
void ArenaGrid::transformArenaWithPreset(const std::vector<std::string>& wallMask) {
	clearArena();

	const int playH = getPlayHeight();
	const int playW = getPlayWidth();
	const int rows = std::min(playH, static_cast<int>(wallMask.size()));

	for (int y = 0; y < rows; ++y) {
		const int cols = std::min(playW, static_cast<int>(wallMask[y].size()));
		for (int x = 0; x < cols; ++x) {
			if (wallMask[y][x] == '#') {
				setCell(x, y, CellType::SpawningSolid);
			}
		}
	}
}

// outline extraction
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
		if (r < 0 || r >= _gridHeight || c < 0 || c >= _gridWidth) return true;
		return _grid[r][c] != CellType::Empty;
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
	for (int r = 0; r < _gridHeight; ++r) {
		for (int c = 0; c < _gridWidth; ++c) {
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

// spawn timing
void ArenaGrid::beginSpawn(float solidifyDelay) {
	_spawnTimer    = solidifyDelay;
	_spawnDuration = solidifyDelay;
}

void ArenaGrid::tickSpawnTimer(float dt) {
	if (_spawnTimer > 0.0f) {
		_spawnTimer -= dt;
		if (_spawnTimer <= 0.0f) {
			for (int y = 1; y < _gridHeight - 1; ++y)
				for (int x = 1; x < _gridWidth - 1; ++x)
					if (_grid[y][x] == CellType::SpawningSolid)
						_grid[y][x] = CellType::Obstacle;
			_fadeTimer = _fadeDuration;
		}
		return;
	}
	if (_fadeTimer > 0.0f)
		_fadeTimer -= dt;
}

bool ArenaGrid::isSpawning() const {
	return _spawnTimer > 0.0f || _fadeTimer > 0.0f;
}

float ArenaGrid::getSpawnFadeProgress() const {
	if (_fadeDuration <= 0.0f || _fadeTimer <= 0.0f) return 1.0f;
	return 1.0f - (_fadeTimer / _fadeDuration);
}

// despawn timing
void ArenaGrid::beginDespawn(float delay) {
	for (int y = 1; y < _gridHeight - 1; ++y)
		for (int x = 1; x < _gridWidth - 1; ++x)
			if (_grid[y][x] == CellType::Obstacle)
				_grid[y][x] = CellType::DespawningSolid;
	_despawnTimer = delay;
}

void ArenaGrid::tickDespawnTimer(float dt) {
	if (_despawnTimer > 0.0f) {
		_despawnTimer -= dt;
		if (_despawnTimer <= 0.0f)
			_fadeOutTimer = _fadeOutDuration;
		return;
	}
	if (_fadeOutTimer > 0.0f) {
		_fadeOutTimer -= dt;
		if (_fadeOutTimer <= 0.0f) {
			for (int y = 1; y < _gridHeight - 1; ++y)
				for (int x = 1; x < _gridWidth - 1; ++x)
					if (_grid[y][x] == CellType::DespawningSolid)
						_grid[y][x] = CellType::Empty;
		}
	}
}

bool ArenaGrid::isDespawning() const {
	return _despawnTimer > 0.0f || _fadeOutTimer > 0.0f;
}

float ArenaGrid::getDespawnFadeProgress() const {
	if (_fadeOutDuration <= 0.0f || _fadeOutTimer <= 0.0f) return 0.0f;
	return _fadeOutTimer / _fadeOutDuration;
}

void ArenaGrid::setMenuArena() {
	static constexpr int MENU_PLAY_W = 60;
	static constexpr int MENU_PLAY_H = 33;

	const int targetGridWidth  = MENU_PLAY_W + 2;
	const int targetGridHeight = MENU_PLAY_H + 2;

	if (_gridWidth != targetGridWidth || _gridHeight != targetGridHeight) {
		_gridWidth = targetGridWidth;
		_gridHeight = targetGridHeight;
	}

	clearArena();
}

void ArenaGrid::setGameplayArena() {
	if (_gridWidth != _defaultGridWidth || _gridHeight != _defaultGridHeight) {
		_gridWidth = _defaultGridWidth;
		_gridHeight = _defaultGridHeight;
	}

	clearArena();
}