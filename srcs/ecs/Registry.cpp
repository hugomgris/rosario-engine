#include "ecs/Registry.hpp"
#include <algorithm>

Entity Registry::createEntity() {
    Entity entity(static_cast<Entity::ID>(entityCount++));
    entities.push_back(entity);
    return entity;
}

void Registry::destroyEntity(Entity entity) {
    auto it = std::remove(entities.begin(), entities.end(), entity);
    entities.erase(it, entities.end());
}

const std::vector<Entity>& Registry::getEntities() const {
    return entities;
}