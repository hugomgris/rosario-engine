#pragma once

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "ecs/Entity.hpp"

template<typename T>
class ComponentPool {
public:
    void add(Entity entity, T component) {
        const auto id = entity.getID();
        if (entityToIndex.count(id))
            throw std::runtime_error("Component already added to entity");

        entityToIndex[id] = components.size();
        components.push_back(component);
        entityList.push_back(entity);
    }

    T& get(Entity entity) {
        const auto id = entity.getID();
        auto it = entityToIndex.find(id);
        if (it == entityToIndex.end())
            throw std::runtime_error("Entity does not have component");
        return components[it->second];
    }

    const T& get(Entity entity) const {
        const auto id = entity.getID();
        auto it = entityToIndex.find(id);
        if (it == entityToIndex.end())
            throw std::runtime_error("Entity does not have component");
        return components[it->second];
    }

    bool has(Entity entity) const {
        return entityToIndex.count(entity.getID()) > 0;
    }

    void remove(Entity entity) {
        const auto id = entity.getID();
        auto it = entityToIndex.find(id);
        if (it == entityToIndex.end())
            return;

        // Swap with last element to keep the array packed
        const size_t removedIndex = it->second;
        const size_t lastIndex    = components.size() - 1;

        if (removedIndex != lastIndex) {
            components[removedIndex] = components[lastIndex];
            entityList[removedIndex] = entityList[lastIndex];
            // Find which entity owns the last slot and update its index
            for (auto& [eid, idx] : entityToIndex) {
                if (idx == lastIndex) {
                    idx = removedIndex;
                    break;
                }
            }
        }

        components.pop_back();
        entityList.pop_back();
        entityToIndex.erase(it);
    }

    size_t size() const { return components.size(); }

    const std::vector<Entity>& entities() const { return entityList; }

private:
    std::vector<T>                              components;
    std::vector<Entity>                         entityList;    // parallel entity IDs
    std::unordered_map<Entity::ID, size_t>      entityToIndex; // entity ID -> index
};