#include "CollisionEffectDispatcher.hpp"

void CollisionEffectDispatcher::registerEffect(const std::string& name, CollisionEffects::EffectFn fn) {
	_effects[name] = fn;
}

void CollisionEffectDispatcher::execute(const std::string& name,
										Registry& registry,
										Entity subject,
										Entity object,
										const CollisionEffects::EffectContext& ctx) const {
	auto it = _effects.find(name);
	if (it == _effects.end())
		throw std::runtime_error("CollisionEffectDispatcher: unknown effect: " + name);
	it->second(registry, subject, object, ctx);
}

void CollisionEffectDispatcher::registerDefaults() {
	registerEffect("GrowSnake",      CollisionEffects::GrowSnake);
	registerEffect("RelocateFood",   CollisionEffects::RelocateFood);
	registerEffect("IncrementScore", CollisionEffects::IncrementScore);
	registerEffect("KillSnake",      CollisionEffects::KillSnake);
}