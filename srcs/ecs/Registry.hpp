#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include "ecs/Entity.hpp"
#include "ecs/ComponentPool.hpp"

class Registry {
public:
    Registry() = default;
    ~Registry() = default;

    Entity createEntity();
    void destroyEntity(Entity entity);

    template<typename T>
    void addComponent(Entity entity, T component);

    template<typename T>
    T& getComponent(Entity entity);

    template<typename T>
    const T& getComponent(Entity entity) const;

    template<typename T>
    bool hasComponent(Entity entity) const;

    template<typename T>
    void removeComponent(Entity entity);

    const std::vector<Entity>& getEntities() const;

    template<typename... T>
    std::vector<Entity> view() const;

private:
    std::vector<Entity> entities;
    std::unordered_map<std::type_index, std::shared_ptr<void>> componentPools;
    size_t entityCount = 0;

    template<typename T>
    ComponentPool<T>& getPool();

    template<typename T>
    const ComponentPool<T>& getPool() const;
};

template<typename T>
void Registry::addComponent(Entity entity, T component) {
    getPool<T>().add(entity, component);
}

template<typename T>
T& Registry::getComponent(Entity entity) {
    return getPool<T>().get(entity);
}

template<typename T>
const T& Registry::getComponent(Entity entity) const {
    return getPool<T>().get(entity);
}

template<typename T>
bool Registry::hasComponent(Entity entity) const {
    return getPool<T>().has(entity);
}

template<typename T>
void Registry::removeComponent(Entity entity) {
    getPool<T>().remove(entity);
}

template<typename T>
ComponentPool<T>& Registry::getPool() {
    const auto key = std::type_index(typeid(T));
    if (componentPools.find(key) == componentPools.end()) {
        componentPools[key] = std::make_shared<ComponentPool<T>>();
    }
    return *std::static_pointer_cast<ComponentPool<T>>(componentPools[key]);
}

template<typename T>
const ComponentPool<T>& Registry::getPool() const {
    const auto key = std::type_index(typeid(T));
    return *std::static_pointer_cast<ComponentPool<T>>(componentPools.at(key));
}

template<typename... T>
std::vector<Entity> Registry::view() const {
    // Get the pool of the first type — it's always the smallest iterator we need.
    // Then filter by the remaining types. This avoids scanning all entities.
    using FirstType = std::tuple_element_t<0, std::tuple<T...>>;
    const auto key = std::type_index(typeid(FirstType));
    auto it = componentPools.find(key);
    if (it == componentPools.end()) return {};

    const auto& firstPool = *std::static_pointer_cast<ComponentPool<FirstType>>(it->second);
    std::vector<Entity> result;
    result.reserve(firstPool.size());
    for (Entity entity : firstPool.entities()) {
        if ((hasComponent<T>(entity) && ...))
            result.push_back(entity);
    }
    return result;
}