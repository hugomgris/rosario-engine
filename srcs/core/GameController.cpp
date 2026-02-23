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

	/* // DEBUG
	if (_foodTracker >= 5 && _foodTracker < 10) {
		//_state->arena->spawnObstacle(10, 10, 5, 5);	
		_state->arena->transformWallWithPreset(WallPreset::InterLock1); 
	} else if (_foodTracker >= 10) {
		_state->arena->clearArena();
		_foodTracker = 0;
	} */

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
	// Process Snake A inpu
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
		//
		Vec2	foodPos = _state->food->getPosition();

		if (head_A.x == foodPos.x && head_A.y == foodPos.y)
		{
			/* if (_state->audio)
				_state->audio->playSound("sound:ñomñomñomñom"); // TODO: real sound implementation */
				
			_state->snake_A->grow();
			for (int i = 0; i < _state->snake_A->getLength(); i++) {
				auto& seg = _state->snake_A->getSegments()[i];
				_state->arena->setCell(seg.x, seg.y, CellType::Snake_A);
			}
			_state->score++;	// Increment score when food is eaten
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
				/* if (_state->audio)
					_state->audio->playSound("sound:ñomñomñomñom"); // TODO: real sound implementation */
					
				_state->snake_B->grow();
				for (int i = 0; i < _state->snake_B->getLength(); i++) {
					auto& seg = _state->snake_B->getSegments()[i];
					_state->arena->setCell(seg.x, seg.y, CellType::Snake_B);
				}
				_state->scoreB++;	// Increment score when food is eaten
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

		// Cross-collision: check NEXT positions against each other
		// A into B's next head OR current body (excluding tail that will move)
		if (!_state->snake_A->isDead()) {
			// Head-on
			if (nextPos_A.x == nextPos_B.x && nextPos_A.y == nextPos_B.y) {
				_state->snake_A->setAsDead(true);
				_state->snake_B->setAsDead(true);
			} else {
				// A into B's body
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

		// B into A's body
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