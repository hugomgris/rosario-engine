#include "Registry.hpp"
#include <algorithm>

Entity Registry::createEntity() {
    Entity entity(static_cast<Entity::ID>(_entityCount++));
    _entities.push_back(entity);
    return entity;
}

void Registry::destroyEntity(Entity entity) {
    auto it = std::remove(_entities.begin(), _entities.end(), entity);
    _entities.erase(it, _entities.end());
    for (auto& [key, pool] : _componentPools)
        pool->remove(entity);
}

const std::vector<Entity>& Registry::getEntities() const {
    return _entities;
}