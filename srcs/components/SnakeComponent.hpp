#pragma once
#include <deque>
#include "DataStructs.hpp"

enum class BeadType { None, Shield, Speed, Ghost, Ram };

struct Segment {
	Vec2 position;
	BeadType bead = BeadType::None;
};

struct SnakeComponent {
	std::deque<Segment> segments;
	bool growing = false;
};