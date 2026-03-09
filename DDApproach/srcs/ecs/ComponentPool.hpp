#pragma once

#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>

class Component {
public:
    virtual ~Component() = default;
};

class ComponentPool {
public:
    template<typename T>
    void addComponent(size_t entityId, T component) {
        auto& componentList = getComponentList<T>();
        if (entityId >= componentList.size()) {
            componentList.resize(entityId + 1);
        }
        componentList[entityId] = std::make_shared<T>(component);
    }

    template<typename T>
    std::shared_ptr<T> getComponent(size_t entityId) {
        auto& componentList = getComponentList<T>();
        if (entityId < componentList.size()) {
            return componentList[entityId];
        }
        return nullptr;
    }

    template<typename T>
    void removeComponent(size_t entityId) {
        auto& componentList = getComponentList<T>();
        if (entityId < componentList.size()) {
            componentList[entityId].reset();
        }
    }

private:
    template<typename T>
    std::vector<std::shared_ptr<T>>& getComponentList() {
        const std::type_index typeIndex = std::type_index(typeid(T));
        if (componentPools.find(typeIndex) == componentPools.end()) {
            componentPools[typeIndex] = std::vector<std::shared_ptr<Component>>();
        }
        return std::any_cast<std::vector<std::shared_ptr<T>>&>(componentPools[typeIndex]);
    }

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Component>>> componentPools;
};