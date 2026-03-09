#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <cstdint>

class Entity {
public:
    using ID = std::uint32_t;

    Entity(ID id) : id(id) {}

    ID getID() const { return id; }

private:
    ID id;
};

#endif // ENTITY_HPP