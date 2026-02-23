#pragma once
#include "DataStructs.hpp"
#include "Utils.hpp"

class Arena;

class Food {
	private:
		Vec2		_position;
		const char	*_foodChar;

	public:
		Food() = delete;
		Food(Vec2 position, int width, int height);
		Food(const Food &other);
		Food &operator=(const Food &other);

	bool replaceInFreeSpace(GameState *gameState);
	void reset(GameState *gameState);  // Reinitialize food at new position

	Vec2 getPosition() const;
	const char* getFoodChar() const;
};