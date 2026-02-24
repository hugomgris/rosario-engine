#include "../incs/GameController.hpp"

GameController::GameController(GameState *state) : _state(state), _foodTracker(0) {}

void GameController::update()  {
	// If AI mode → generate AI decision each frame
	if (_state->config.mode == GameMode::AI && aiController && _state->snake_B) {
		Input aiMove = aiController->decideNextMove(*_state);
		if (aiMove != Input::None) {
			bufferInput(aiMove);
		}
	}
	
	processNextInput();
	
	// Check collisions BEFORE moving
	if (!checkGameOverCollision()) {
		std::cout << "GAME OVER - A dead: " << _state->snake_A->isDead();
		if (_state->snake_B)
			std::cout << " B dead: " << _state->snake_B->isDead();
		std::cout << std::endl;
		_state->isRunning = false;
		_state->currentState = GameStateType::GameOver;
		return;
	}
	
	// Safe to move
	_state->snake_A->move();
	updateSnakeInArena(*_state->snake_A, CellType::Snake_A);
	if (_state->config.mode == GameMode::MULTI && _state->snake_B)
	{
		_state->snake_B->move();
		updateSnakeInArena(*_state->snake_B, CellType::Snake_B);
	}
	if (_state->config.mode == GameMode::AI && _state->snake_B) {
		_state->snake_B->move();
		updateSnakeInArena(*_state->snake_B, CellType::Snake_B);
	}
	
	checkHeadFoodCollision();
}

void GameController::bufferInput(Input input) {
	// Player A (Arrows)
	if (input >= Input::Up_A && input <= Input::Right_A) {
		if (inputBuffer_A.size() < MAX_BUFFER_SIZE) {
			inputBuffer_A.push(input);
		}
	} else if (_state->config.mode == GameMode::MULTI && input >= Input::Up_B && input <= Input::Right_B) { 
		if (inputBuffer_B.size() < MAX_BUFFER_SIZE) {
			inputBuffer_B.push(input); // Player B (WASD)
		}
	} else if (_state->config.mode == GameMode::AI && input >= Input::Up_B && input <= Input::Right_B) {
		// AI moves
		if (inputBuffer_B.size() < MAX_BUFFER_SIZE) {
			inputBuffer_B.push(input);
		}
	}
}

void GameController::processNextInput() {
	// Process Snake A input
	if (!inputBuffer_A.empty()) {
		Input input = inputBuffer_A.front();
		inputBuffer_A.pop();
		
		switch (input) {
			case Input::Up_A:
				_state->snake_A->changeDirection(Direction::Up);
				break;
			case Input::Down_A:
				_state->snake_A->changeDirection(Direction::Down);
				break;
			case Input::Left_A:
				_state->snake_A->changeDirection(Direction::Left);
				break;
			case Input::Right_A:
				_state->snake_A->changeDirection(Direction::Right);
				break;
			default:
				break;
		}
	}
	
	// Process Snake B input
	if (!inputBuffer_B.empty()) {
		Input input = inputBuffer_B.front();
		inputBuffer_B.pop();
		
		switch (input) {
			case Input::Up_B:
				if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
					_state->snake_B->changeDirection(Direction::Up);
				}
				break;
			case Input::Down_B:
				if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
					_state->snake_B->changeDirection(Direction::Down);
				}
				break;
			case Input::Left_B:
				if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
					_state->snake_B->changeDirection(Direction::Left);
				}
				break;
			case Input::Right_B:
				if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
					_state->snake_B->changeDirection(Direction::Right);
				}
				break;
			default:
				break;
		}
	}
}

void GameController::checkHeadFoodCollision() {
	Vec2	head_A = _state->snake_A->getSegments()[0];
	Vec2	foodPos = _state->food->getPosition();

	if (head_A.x == foodPos.x && head_A.y == foodPos.y)
	{
		_state->snake_A->grow();
		for (int i = 0; i < _state->snake_A->getLength(); i++) {
			auto& seg = _state->snake_A->getSegments()[i];
			_state->arena->setCell(seg.x, seg.y, CellType::Snake_A);
		}
		_state->score++;
		_foodTracker++;

		if (_foodTracker == 2) {
			if (_state->arena->isDespawning() || _state->arena->isSpawning()) return;
			_state->arena->transformArenaWithPreset(ArenaPresets::getRandomPreset());
			updateSnakeInArena(*_state->snake_A, CellType::Snake_A);
			if (_state->snake_B)
				updateSnakeInArena(*_state->snake_B, CellType::Snake_B);
			if (onArenaChangeSpawnCallBack)
				onArenaChangeSpawnCallBack();
		} else if (_foodTracker >= 6) {
			if (_state->arena->isDespawning() || _state->arena->isSpawning()) return;
			if (onArenaClearCallBack)
				onArenaClearCallBack();
			_foodTracker = 0;
		}
		
		if (!_state->food->replaceInFreeSpace(_state)) {
			_state->isRunning = false;
			std::cout << "YOU WIN" << std::endl;
		}
	}

	if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
		Vec2	head_B = _state->snake_B->getSegments()[0];

		if (head_B.x == foodPos.x && head_B.y == foodPos.y) {
			_state->snake_B->grow();
			for (int i = 0; i < _state->snake_B->getLength(); i++) {
				auto& seg = _state->snake_B->getSegments()[i];
				_state->arena->setCell(seg.x, seg.y, CellType::Snake_B);
			}
			_state->scoreB++;
			_foodTracker++;

			if (_foodTracker == 2) {
				if (_state->arena->isDespawning() || _state->arena->isSpawning()) return;
				_state->arena->transformArenaWithPreset(ArenaPresets::getRandomPreset());
				updateSnakeInArena(*_state->snake_A, CellType::Snake_A);
				if (_state->snake_B)
					updateSnakeInArena(*_state->snake_B, CellType::Snake_B);
				if (onArenaChangeSpawnCallBack)
					onArenaChangeSpawnCallBack();
			} else if (_foodTracker >= 6) {
				if (_state->arena->isDespawning() || _state->arena->isSpawning()) return;
				if (onArenaClearCallBack)
					onArenaClearCallBack();
				_foodTracker = 0;
			}
							
			if (!_state->food->replaceInFreeSpace(_state)) {
				_state->isRunning = false;
				std::cout << "YOU WIN" << std::endl;
			}
		}
	}
}

bool GameController::checkGameOverCollision() {
	Vec2 nextPos_A = _state->snake_A->getNextHeadPosition();
	bool snakeB_active = (_state->config.mode != GameMode::SINGLE && _state->snake_B);
	Vec2 nextPos_B = snakeB_active ? _state->snake_B->getNextHeadPosition() : Vec2{-1,-1};

	// Check A
	CellType cellA = _state->arena->getCell(nextPos_A.x, nextPos_A.y);
	if (cellA == CellType::Wall || cellA == CellType::Obstacle || cellA == CellType::DespawningSolid)
		_state->snake_A->setAsDead(true);

	// Snake A into self
	if (!_state->snake_A->isDead()) {
		int limit = _state->snake_A->getIsGrowing()
			? _state->snake_A->getLength()
			: _state->snake_A->getLength() - 1;
		for (int i = 1; i < limit; i++) {
			auto& seg = _state->snake_A->getSegments()[i];
			if (seg.x == nextPos_A.x && seg.y == nextPos_A.y) {
				_state->snake_A->setAsDead(true);
				break;
			}
		}
	}

	if (snakeB_active) {
		CellType cellB = _state->arena->getCell(nextPos_B.x, nextPos_B.y);
		if (cellB == CellType::Wall || cellB == CellType::Obstacle || cellB == CellType::DespawningSolid)
			_state->snake_B->setAsDead(true);

		// Snake B into self
		if (!_state->snake_B->isDead()) {
			int limit = _state->snake_B->getIsGrowing()
				? _state->snake_B->getLength()
				: _state->snake_B->getLength() - 1;
			for (int i = 1; i < limit; i++) {
				auto& seg = _state->snake_B->getSegments()[i];
				if (seg.x == nextPos_B.x && seg.y == nextPos_B.y) {
					_state->snake_B->setAsDead(true);
					break;
				}
			}
		}

		if (!_state->snake_A->isDead()) {
			if (nextPos_A.x == nextPos_B.x && nextPos_A.y == nextPos_B.y) {
				_state->snake_A->setAsDead(true);
				_state->snake_B->setAsDead(true);
			} else {
				int limit = _state->snake_B->getIsGrowing()
					? _state->snake_B->getLength()
					: _state->snake_B->getLength() - 1;
				for (int i = 0; i < limit; i++) {
					auto& seg = _state->snake_B->getSegments()[i];
					if (seg.x == nextPos_A.x && seg.y == nextPos_A.y) {
						_state->snake_A->setAsDead(true);
						break;
					}
				}
			}
		}

		if (!_state->snake_B->isDead()) {
			int limit = _state->snake_A->getIsGrowing()
				? _state->snake_A->getLength()
				: _state->snake_A->getLength() - 1;
			for (int i = 0; i < limit; i++) {
				auto& seg = _state->snake_A->getSegments()[i];
				if (seg.x == nextPos_B.x && seg.y == nextPos_B.y) {
					_state->snake_B->setAsDead(true);
					break;
				}
			}
		}
	}

	return !(_state->snake_A->isDead() || (snakeB_active && _state->snake_B->isDead()));
}

void GameController::clearInputBuffer() {
	while (!inputBuffer_A.empty()) {
		inputBuffer_A.pop();
	}
	while (!inputBuffer_B.empty()) {
		inputBuffer_B.pop();
	}
}

void GameController::setAIController(SnakeAI *ai) {
	aiController = ai;
}

void GameController::updateSnakeInArena(Snake& snake, CellType type) {
	auto& arena = *_state->arena;
	arena.setCell(snake.getHead().x, snake.getHead().y, type);
	if (snake.didRemoveTail()) {
		Vec2 tail = snake.getDroppedTail();
		arena.clearCell(tail.x, tail.y);
	}
}

// ---------------------------------------------------------------------------
// Solidification handler — called by Arena::tickSpawnTimer the instant
// SpawningSolid cells become Obstacle.
//
// For each snake, walks segments from head (index 0) forward to find the
// first segment sitting on a newly-solidified cell.  Everything from that
// index to the tail is destroyed: the arena cells are cleared, a blue
// particle explosion fires on every destroyed segment, and the snake is
// truncated.  If the head itself is the cut point the snake is marked dead
// and the normal game-over path handles the rest next tick.
// ---------------------------------------------------------------------------
void GameController::onSolidify(const std::vector<std::pair<int,int>>& solidifiedPositions,
								ParticleSystem& particles,
								const Renderer& renderer)
{
	// Build a fast O(log n) lookup from the solidified game-coordinate list.
	std::set<std::pair<int,int>> solidSet(solidifiedPositions.begin(),
										solidifiedPositions.end());

	// Vivid steel-blue, matching the spawning visual language.
	const Color blueExplosion = {70, 130, 180, 255};

	auto truncateSnake = [&](Snake* snake) {
		if (!snake) return;

		const Vec2* segs = snake->getSegments();
		int len = snake->getLength();

		// Find the first segment that landed on a solidified cell.
		int cutIndex = -1;
		for (int i = 0; i < len; i++) {
			if (solidSet.count({segs[i].x, segs[i].y})) {
				cutIndex = i;
				break;
			}
		}

		if (cutIndex < 0) return;	// snake is entirely clear — nothing to do

		// Blast particles and clean up all condemned segments.
		for (int i = cutIndex; i < len; i++) {
			Vector2 screenPos = renderer.gridToScreen2D(segs[i].x, segs[i].y);
			float cx = screenPos.x + renderer.getSquareSize() * 0.5f;
			float cy = screenPos.y + renderer.getSquareSize() * 0.5f;

			particles.spawnExplosion(cx, cy, 12);

			// The cut-point cell is already Obstacle (solidified before this
			// callback fired) — leave it alone.
			// Body segments BEHIND the cut point are on normal cells still
			// typed Snake_A/B; overwrite them to Empty directly.  We must NOT
			// call clearCell() here because clearCell() has special restoration
			// logic that would check _occupiedSpawning (now already cleared) and
			// fall through to setting Obstacle incorrectly for non-solid cells.
			if (i > cutIndex) {
				_state->arena->setCell(segs[i].x, segs[i].y, CellType::Empty);
			}
		}

		if (cutIndex == 0) {
			// Head was crushed — mark dead and let the game-over path take over.
			snake->setAsDead(true);
		} else {
			// Shrink the snake to the surviving prefix.
			snake->truncateTo(cutIndex);
		}
	};

	truncateSnake(_state->snake_A);
	if (_state->config.mode != GameMode::SINGLE)
		truncateSnake(_state->snake_B);
}