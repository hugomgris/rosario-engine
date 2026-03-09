#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include "ecs/Entity.hpp"
#include "ecs/ComponentPool.hpp"

class Registry {
	private:
		std::vector<Entity>											_entities;
		std::unordered_map<std::type_index, std::shared_ptr<void>>	_componentPools;
		size_t														_entityCount = 0;
		
		template<typename T>
		ComponentPool<T>& getPool();

		template<typename T>
		const ComponentPool<T>& getPool() const;

	public:
		Registry() = default;
		~Registry() = default;

		Entity createEntity();
		void destroyEntity(Entity entity);

		template<typename T>
		void addComponent(Entity entity, T component);

		template<typename T>
		T& getComponent(Entity entity) const;

		template<typename T>
		bool hasComponent(Entity entity) const;

		template<typename T>
		void removeComponent(Entity entity);

		const std::vector<Entity>& getEntities() const;
		
		template<typename... T>
		std::vector<Entity> view() const;
};

template<typename T>
void Registry::addComponent(Entity entity, T component) {
	getPool<T>().add(entity, component);
}

template<typename T>
T& Registry::getComponent(Entity entity) const {
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
ComponentPool<T>& Registry::getPool(){
	const auto key = std::type_index(typeid(T));
	if (_componentPools.find(key) == _componentPools.end()) {
		_componentPools[key] = std::make_shared<ComponentPool<T>>();
	}
	return *std::static_pointer_cast<ComponentPool<T>>(_componentPools[key]);
}

template<typename T>
const ComponentPool<T>& Registry::getPool() const {
	const auto key = std::type_index(typeid(T));
	return *std::static_pointer_cast<ComponentPool<T>>(_componentPools.at(key));
}

// the main, entity polling function in this registry
template<typename... T>
std::vector<Entity> Registry::view() const {
	// get the pool of the first type, which is always the smallest iterator needed
	// then, filter by the remaining types to avoid scanning all entities
	using FirstType = std::typle_element_t<0, std::type<T...>>;
	const auto key = std::type_index(typeid(FirstType));
	auto it = _componentPools.find(key);
	if (it == _componentPools.end()) return {};

	const auto& firstPool = *std::static_pointer_cast<ComponentPool<FirstType>>(it->second);
	std::vector<Entity> result;
	result.reserve(firstPool.size());
	for (Entity entity : firstPool.entities()) {
		if (hasComponent<T>(entity) && ...)
			result.push_back(entity);
	}

	return result;
}