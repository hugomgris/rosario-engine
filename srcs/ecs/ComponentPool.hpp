#pragma once

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "ecs/Entity.hpp"

template<typename T>
class ComponentPool {
	private:
		std::vector<T>							_components;
		std::vector<Entity>						_entityList;
		std::unordered_map<Entity::ID, size_t>	_entityToIndex;

	public:
		void add(Entity entity, T component) {
			const auto id = entity.getID();
			if (_entityToIndex.count(id))
				throw std::runtime_error("Component already added to entity");

			_entityToIndex[id] = _components.size();
			_components.push_back(component);
			_entityList.push_back(entity);
		}

		T& get(Entity entity) {
			const auto id = entity.getID();
			auto it = _entityToIndex.find(id);
			if (it == _entityToIndex.end())
				throw std::runtime_error("Entity does not have component");
			return _components[it->second];
		}

		const T& get(Entity entity) const {
			const auto id = entity.getID();
			auto it = _entityToIndex.find(id);
			if (it == _entityToIndex.end())
				throw std::runtime_error("Entity does not have component");
			return _components[it->second];
		}

		bool has(Entity entity) const {
			return _entityToIndex.count(entity.getID()) > 0;
		}

		void remove(Entity entity) {
			const auto id = entity.getID();
			auto it = _entityToIndex.find(id);
			if (it == _entityToIndex.end())
				return;

			// swap with last element to keep the array packed
			const size_t removedIndex	= it->second;
			const size_t lastIndex 		= _components.size() - 1;

			if (removedIndex != lastIndex) {
				_components[removedIndex] = _components[lastIndex];
				_entityList[removedIndex] = _entityList[lastIndex];

				// find which entity owns the last slot and update its index
				for (auto& [eid, idx] : _entityToIndex) {
					if (idx == lastIndex) {
						idx = removedIndex;
						break;
					}
				}
			}

			_components.pop_back();
			_entityList.pop_back();
			_entityToIndex.erase(it);
		}

		size_t size() const { return _components.size(); }

		const std::vector<Entity>& entities() const { return _entityList; }
};
