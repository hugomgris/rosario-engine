#include "Registry.hpp"
#include "Entity.hpp"
#include "ComponentPool.hpp"
#include <algorithm>

Registry::Registry() : entityCount(0) {}

Entity Registry::createEntity() {
    Entity entity(entityCount++);
    entities.push_back(entity);
    return entity;
}

void Registry::destroyEntity(Entity entity) {
    auto it = std::remove(entities.begin(), entities.end(), entity);
    entities.erase(it, entities.end());
    // Optionally clear components associated with the entity
    clearEntityComponents(entity);
}

void Registry::clearEntityComponents(Entity entity) {
    for (auto& pool : componentPools) {
        pool->remove(entity);
    }
}

template<typename T>
void Registry::registerComponent() {
    const char* typeName = typeid(T).name();
    if (componentPools.find(typeName) == componentPools.end()) {
        componentPools[typeName] = std::make_unique<ComponentPool<T>>();
    }
}

template<typename T>
void Registry::addComponent(Entity entity, T component) {
    const char* typeName = typeid(T).name();
    if (componentPools.find(typeName) != componentPools.end()) {
        auto pool = static_cast<ComponentPool<T>*>(componentPools[typeName].get());
        pool->add(entity, component);
    }
}

template<typename T>
T& Registry::getComponent(Entity entity) {
    const char* typeName = typeid(T).name();
    if (componentPools.find(typeName) != componentPools.end()) {
        auto pool = static_cast<ComponentPool<T>*>(componentPools[typeName].get());
        return pool->get(entity);
    }
    throw std::runtime_error("Component not found");
}

template<typename T>
void Registry::removeComponent(Entity entity) {
    const char* typeName = typeid(T).name();
    if (componentPools.find(typeName) != componentPools.end()) {
        auto pool = static_cast<ComponentPool<T>*>(componentPools[typeName].get());
        pool->remove(entity);
    }
}

std::vector<Entity> Registry::getEntities() const {
    return entities;
}