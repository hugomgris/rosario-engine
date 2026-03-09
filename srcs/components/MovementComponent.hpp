#pragma once
#include "DataStructs.hpp"

struct MovementComponent {
	Direction direction;
	float moveTimer;
	float moveInterval;
};