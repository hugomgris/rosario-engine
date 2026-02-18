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
		_state->isRunning = false;
		_state->currentState = GameStateType::GameOver;
		return;  // Don't move if collision detected
	}
	
	// Safe to move
	_state->snake_A->move();
	if (_state->config.mode == GameMode::MULTI && _state->snake_B)
		_state->snake_B->move();
	if (_state->config.mode == GameMode::AI && _state->snake_B)
		_state->snake_B->move();
	
	checkHeadFoodCollision();

	// DEBUG AND TEST
	if (_foodTracker > 0 && _foodTracker % 3 == 0) {
		_state->arena->spawnObstacle(10, 10, 5, 5);	
	} else if (_foodTracker > 0 && _foodTracker % 5 == 0) {
		_state->arena->clearArena();
	}
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
		_state->score++;  // Increment score when food is eaten
		_foodTracker++;
		
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
			_state->scoreB++;  // Increment score when food is eaten
			
			if (!_state->food->replaceInFreeSpace(_state)) {
				_state->isRunning = false;
				std::cout << "YOU WIN" << std::endl;
			}
		}
	}
}

// TODO: handle snake B collision with obstacles and growths
bool GameController::checkGameOverCollision()
{
	// Get next positions based on current direction
	Vec2 head_A = _state->snake_A->getSegments()[0];
	Vec2 nextPos_A = _state->snake_A->getNextHeadPosition();
	
	// Check if next position is a wall or obstacle
	if (_state->arena->getCell(nextPos_A.x, nextPos_A.y) == CellType::Wall ||
			_state->arena->getCell(nextPos_A.x, nextPos_A.y) == CellType::Obstacle) {
		_state->snake_A->setAsDead(true);
		std::cout << "Snake A DIED against a wall or obstacle" << std::endl;
		return false;
	}
	
	// Check self-collision for snake A 
	for (int i = 1; i < _state->snake_A->getLength() - 1; i++)
	{
		if (_state->snake_A->getSegments()[i].x == nextPos_A.x && 
				_state->snake_A->getSegments()[i].y == nextPos_A.y) {
			_state->snake_A->setAsDead(true);
			std::cout << "Snake A DIED against itself" << std::endl;
		}
	}

	if (_state->snake_A->isDead()) {
		return false;
	}

	if (_state->config.mode != GameMode::SINGLE && _state->snake_B) {
		Vec2 head_B = _state->snake_B->getSegments()[0];
		Vec2 nextPos_B = _state->snake_B->getNextHeadPosition();

		// Check if next position is a wall
		if (_state->arena->getCell(nextPos_B.x, nextPos_B.y) == CellType::Wall) {
			_state->snake_B->setAsDead(true);
			std::cout << "Snake B DIED against a wall" << std::endl;
			return false;
		}

		// Check if snake_A's next position collides with snake_B's body 
		for (int i = 0; i < _state->snake_B->getLength() - 1; i++)
		{
			if (_state->snake_B->getSegments()[i].x == nextPos_A.x && 
					_state->snake_B->getSegments()[i].y == nextPos_A.y) {
				_state->snake_A->setAsDead(true);
				std::cout << "Snake A DIED against B snake" << std::endl;
			}
		}

		// Check self-collision for snake B 
		for (int i = 1; i < _state->snake_B->getLength() - 1; i++)
		{
			if (_state->snake_B->getSegments()[i].x == nextPos_B.x && 
					_state->snake_B->getSegments()[i].y == nextPos_B.y) {
				_state->snake_B->setAsDead(true);
				std::cout << "Snake B DIED against itself" << std::endl;
			}
		}

		if (_state->snake_B->isDead()) {
			return false;
		}

		// Check if snake_B's next position collides with snake_A's body 
		for (int i = 0; i < _state->snake_A->getLength() - 1; i++)
		{
			if (_state->snake_A->getSegments()[i].x == nextPos_B.x && 
					_state->snake_A->getSegments()[i].y == nextPos_B.y) {
				_state->snake_B->setAsDead(true);
				std::cout << "Snake B DIED against A snake" << std::endl;
			}
		}

		if (_state->snake_A->isDead() || _state->snake_B->isDead()) {
			return false;
		}
	}

	return true;
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