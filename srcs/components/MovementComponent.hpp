#pragma once

#include "../../incs/DataStructs.hpp"

struct MovementComponent {
	Direction	direction;
	float		moveTimer;
	float		moveInterval;
};