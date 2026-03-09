#pragma once

#include <cstdint>

class Entity {
public:
    using ID = std::uint32_t;

    explicit Entity(ID id) : id(id) {}

    ID getID() const { return id; }

    bool operator==(const Entity& other) const { return id == other.id; }
    bool operator!=(const Entity& other) const { return id != other.id; }

private:
    ID id;
};