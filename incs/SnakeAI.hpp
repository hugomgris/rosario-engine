#pragma once

#include "DataStructs.hpp"
#include "Pathfinder.hpp"
#include "FloodFill.hpp"
#include "Input.hpp"

class SnakeAI {
public:
	SnakeAI(AIConfig config);
	
	Input decideNextMove(const GameState& state);
	
	private:
		AIConfig config;
		PathFinder pathFinder;
		FloodFill floodFill;
		
		// decision modes
		Input goToFood(const GameState& state);
		Input survivalMode(const GameState& state);
		Input maximizeSpace(const GameState& state);
		
		// safety checks
		bool isSafeMove(const GameState& state, Vec2 nextPos);
		int evaluateMoveSpace(const GameState& state, Vec2 nextPos);
};