#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include "Entity.hpp"
#include "ComponentPool.hpp"

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
    bool hasComponent(Entity entity) const;

    template<typename T>
    void removeComponent(Entity entity);

private:
    std::unordered_map<Entity, std::unordered_map<std::type_index, std::shared_ptr<void>>> entityComponents;
    ComponentPool componentPool;
};

template<typename T>
void Registry::addComponent(Entity entity, T component) {
    auto componentPtr = std::make_shared<T>(component);
    entityComponents[entity][std::type_index(typeid(T))] = componentPtr;
    componentPool.addComponent(entity, component);
}

template<typename T>
T& Registry::getComponent(Entity entity) {
    return *std::static_pointer_cast<T>(entityComponents[entity][std::type_index(typeid(T))]);
}

template<typename T>
bool Registry::hasComponent(Entity entity) const {
    return entityComponents.at(entity).count(std::type_index(typeid(T))) > 0;
}

template<typename T>
void Registry::removeComponent(Entity entity) {
    entityComponents[entity].erase(std::type_index(typeid(T)));
    componentPool.removeComponent(entity);
}

#endif // REGISTRY_HPP