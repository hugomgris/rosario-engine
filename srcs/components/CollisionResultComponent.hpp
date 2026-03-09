#pragma once

// Written by CollisionSystem each frame.
// Read by the game controller to decide what to do (kill snake, grow it, respawn food...).
// Cleared at the start of each CollisionSystem update so stale results don't linger.
enum class CollisionType {
    None,
    Wall,       // Head hit a SolidTag entity
    Self,       // Head hit own body
    Snake,      // Head hit another snake's body
    Food,       // Head hit a FoodTag entity
};

struct CollisionResultComponent {
    CollisionType result = CollisionType::None;
};
