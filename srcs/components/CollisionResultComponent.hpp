#pragma once

enum class CollisionType {
	None,
	Wall,
	Self,
	Snake,
	Food,
};

struct CollisionResultComponent {
	CollisionType result = CollisionType::None;
};