#include "../incs/Snake.hpp"
#include <iostream>

Snake::Snake(int width, int height): _length(4), _maxLength((width * height) - 2) {
	_segments = new Vec2[_maxLength];
	initializeAtRandomPosition(width, height);
}

void Snake::initializeAtRandomPosition(int width, int height) {
	_length = 4;
	_isDead = false;
	
	switch (Utils::getRandomInt(3))
	{
		case 0:
			_direction = Direction::Up;
			break;
		case 1:
			_direction = Direction::Down;
			break;
		case 2:
			_direction = Direction::Left;
			break;
		case 3:
			_direction = Direction::Right;
			break;
	}

	Vec2 headPosition = Utils::getRandomSpawnPosition(width, height);

	switch (_direction) {
		case Direction::Up:
			_segments[0] = headPosition;
			_segments[1] = { headPosition.x, headPosition.y + 1 };
			_segments[2] = { headPosition.x, headPosition.y + 2 };
			_segments[3] = { headPosition.x, headPosition.y + 3 };
			break;

		case Direction::Down:
			_segments[0] = headPosition;
			_segments[1] = { headPosition.x, headPosition.y - 1 };
			_segments[2] = { headPosition.x, headPosition.y - 2 };
			_segments[3] = { headPosition.x, headPosition.y - 3 };
			break;

		case Direction::Left:
			_segments[0] = headPosition;
			_segments[1] = { headPosition.x + 1, headPosition.y };
			_segments[2] = { headPosition.x + 2, headPosition.y };
			_segments[3] = { headPosition.x + 3, headPosition.y };
			break;

		case Direction::Right:
			_segments[0] = headPosition;
			_segments[1] = { headPosition.x - 1, headPosition.y };
			_segments[2] = { headPosition.x - 2, headPosition.y };
			_segments[3] = { headPosition.x - 3, headPosition.y };
			break;
	}

	// DEBUG
	//std::cout << "head snake coords:" << _segments[0].x << "-" << _segments[0].y << std::endl;
}

Snake::Snake(const Snake &otherSnake, int width, int height) : _length(otherSnake._length), _maxLength(otherSnake._maxLength) {
	_segments = new Vec2[_maxLength];
	initializeAsMirrored(otherSnake, width, height);
}

void Snake::initializeAsMirrored(const Snake &otherSnake, int width, int height) {
	_length = 4;
	_isDead = false;
	
	switch (otherSnake._direction)
	{
		case Direction::Up:
			_direction = Direction::Down;
			break;
		case Direction::Down:
			_direction = Direction::Up;
			break;
		case Direction::Left:
			_direction = Direction::Right;
			break;
		case Direction::Right:
			_direction = Direction::Left;
			break;
	}

	_segments[0].x = width - 1 - otherSnake._segments[0].x;
	_segments[0].y = height - 1 - otherSnake._segments[0].y;
	_segments[1].x = width - 1 - otherSnake._segments[1].x;
	_segments[1].y = height - 1 - otherSnake._segments[1].y;
	_segments[2].x = width - 1 - otherSnake._segments[2].x;
	_segments[2].y = height - 1 - otherSnake._segments[2].y;
	_segments[3].x = width - 1 - otherSnake._segments[3].x;
	_segments[3].y = height - 1 - otherSnake._segments[3].y;
}

Snake::Snake(Snake &&other) noexcept : _length(other._length), _maxLength(other._maxLength), _segments(other._segments), _direction(other._direction) {
	other._segments = nullptr;
}

Snake &Snake::operator=(Snake &&other) noexcept {
	if (this != &other)
	{
		delete[] _segments;
		
		this->_length = other._length;
		this->_maxLength = other._maxLength;
		this->_direction = other._direction;
		this->_segments = other._segments;
		
		other._segments = nullptr;
	}
	return *this;
}

Snake::Snake(const Snake &other) : _length(other._length), _maxLength(other._maxLength) {
	_segments = new Vec2[_maxLength];
	_direction = other._direction;
	for (int i = 0; i < _length; ++i) {
		_segments[i] = other._segments[i];
	}
}

Snake &Snake::operator=(const Snake &other) {
	if (this != &other)
	{
		delete[] _segments;
		
		this->_length = other._length;
		this->_maxLength = other._maxLength;
		this->_direction = other._direction;
		this->_segments = new Vec2[_maxLength];
		
		for (int i = 0; i < this->_length; ++i)
			this->_segments[i] = other._segments[i];
	}
	return *this;
}

Snake::~Snake() {
	delete[] _segments;
}

void Snake::setAsDead(bool dead) { _isDead = dead; }

bool Snake::isDead() const { return _isDead; }

int Snake::getLength() const { return _length; }

const Vec2 *Snake::getSegments() const { return _segments; }

const Vec2& Snake::getHead() const { return _segments[0]; }

Direction Snake::getDirection() const { return _direction; }

bool Snake::didRemoveTail() const { return _didRemoveTail; }
Vec2 Snake::getDroppedTail() const { return _lastDroppedTail; }

bool Snake::getIsGrowing() const { return _isGrowing; }

void Snake::move(){
	auto head = _segments[0];
	Vec2 previousPositions[_length];
	for (int i = 0; i < _length; i++) {
		previousPositions[i] = _segments[i];
	}
	
	switch (_direction)
	{
		case Direction::Left:
			head.x--;
			break;

		case Direction::Right:
			head.x++;
			break;

		case Direction::Up:
			head.y--;
			break;

		case Direction::Down:
			head.y++;
			break;
	}

	_segments[0] = head;

	for (int i = 1; i < _length; ++i)
	{
		Vec2 newPos;
		newPos.x = previousPositions[i - 1].x;
		newPos.y = previousPositions[i - 1].y;
		_segments[i] = newPos;
	}

	// After moving, handle tail removal
	if (!_isGrowing) {
		_didRemoveTail = true;
		_lastDroppedTail = previousPositions[_length - 1];
	} else {
		_didRemoveTail = false;
	}
	_isGrowing = false;
}

void Snake::changeDirection(Direction dir) { 
	if ((_direction == Direction::Up && dir == Direction::Down)
		|| (_direction == Direction::Down && dir == Direction::Up)
		|| (_direction == Direction::Left && dir == Direction::Right)
		|| (_direction == Direction::Right && dir == Direction::Left))
	{
		return;
	}

	_direction = dir;
};

void Snake::grow() {
	if (_length >= _maxLength) {
		// Snake has filled the entire arena, which should trigger a win condition (will this ever happen to anyone tho?)
		return;
	}
	_segments[_length] = Vec2{ _segments[_length - 1].x, _segments[_length - 1].y };
	_length++;
	_isGrowing = true;
}

void Snake::reset(int width, int height) {
	initializeAtRandomPosition(width, height);
}

void Snake::resetAsMirrored(const Snake& otherSnake, int width, int height) {
	initializeAsMirrored(otherSnake, width, height);
}

Vec2 Snake::getNextHeadPosition() const {
	Vec2 nextPos = _segments[0];
	switch (_direction) {
		case Direction::Left:
			nextPos.x--;
			break;
		case Direction::Right:
			nextPos.x++;
			break;
		case Direction::Up:
			nextPos.y--;
			break;
		case Direction::Down:
			nextPos.y++;
			break;
	}
	return nextPos;
}

