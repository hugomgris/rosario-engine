#pragma once

#include "ecs/Registry.hpp"
#include "DataStructs.hpp"

// Maps a logical player identity to the input enum values assigned to them.
// The InputSystem uses this to know which Raylib keys to poll for each entity.
enum class PlayerSlot {
    A,  // WASD
    B,  // Arrow keys
};

class InputSystem {
public:
    void update(Registry& registry);

    // Called from main (or a factory) when creating a snake entity, to tell
    // the InputSystem which player slot that entity belongs to.
    void assignSlot(Entity entity, PlayerSlot slot);

private:
    void pollPlayerA(Registry& registry, Entity entity);
    void pollPlayerB(Registry& registry, Entity entity);

    std::unordered_map<Entity::ID, PlayerSlot> slotMap;
};
