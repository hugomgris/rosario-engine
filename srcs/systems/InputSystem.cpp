#include <raylib.h>
#include "InputSystem.hpp"
#include "../components/InputComponent.hpp"

void InputSystem::assignSlot(Entity entity, PlayerSlot slot) {
	slotMap[entity.getID()] = slot;
}

// p1 -> WASD

void InputSystem::pollPlayerA(Registry& registry, Entity entity) {
	auto& input = registry.getComponent<InputComponent>(entity);

	if (IsKeyPressed(KEY_UP))    input.inputBuffer.push(Input::Up_B);
	if (IsKeyPressed(KEY_DOWN))  input.inputBuffer.push(Input::Down_B);
	if (IsKeyPressed(KEY_LEFT))  input.inputBuffer.push(Input::Left_B);
	if (IsKeyPressed(KEY_RIGHT)) input.inputBuffer.push(Input::Right_B);
}

// p2 -> arrows
void InputSystem::pollPlayerB(Registry& registry, Entity entity) {
	auto& input = registry.getComponent<InputComponent>(entity);

	if (IsKeyPressed(KEY_W))     input.inputBuffer.push(Input::Up_A);
	if (IsKeyPressed(KEY_S))     input.inputBuffer.push(Input::Down_A);
	if (IsKeyPressed(KEY_A))     input.inputBuffer.push(Input::Left_A);
	if (IsKeyPressed(KEY_D))     input.inputBuffer.push(Input::Right_A);
}

// iterates over all entities with an InputComponent ad polls raylib
// no assigne dslot = skipped
void InputSystem::update(Registry& registry) {
	for (auto entity : registry.view<InputComponent>()) {
		auto it = slotMap.find(entity.getID());
		if (it == slotMap.end())
			continue;

		switch (it->second) {
			case PlayerSlot::A: pollPlayerA(registry, entity); break;
			case PlayerSlot::B: pollPlayerB(registry, entity); break;
		}
	}
}