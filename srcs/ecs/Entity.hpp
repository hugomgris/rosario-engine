#pragma once

#include <cstdint>

// Simple entity class: just an ID (number) with necessary == and != operators
class Entity {
	using ID = std::uint32_t;

	private:
		ID _id;
	
	public:
		explicit Entity(ID id) : _id(id) {}

		ID getID() const { return _id; }

		bool operator==(const Entity& other) const { return _id == other._id; }
		bool operator!=(const Entity& other) const { return _id != other._id; }
};