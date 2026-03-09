#include "ParticleSystem.hpp"
#include "ecs/Registry.hpp"
#include "components/TransformComponent.hpp"
#include "components/RenderComponent.hpp"
#include <vector>

ParticleSystem::ParticleSystem(Registry& registry) : registry(registry) {}

void ParticleSystem::update(float deltaTime) {
    for (auto entity : registry.getEntitiesWith<ParticleComponent>()) {
        auto& particle = registry.getComponent<ParticleComponent>(entity);
        auto& transform = registry.getComponent<TransformComponent>(entity);
        
        // Update particle position based on velocity
        transform.position += particle.velocity * deltaTime;

        // Update particle lifetime
        particle.lifetime -= deltaTime;
        if (particle.lifetime <= 0) {
            registry.destroyEntity(entity);
        }
    }
}

void ParticleSystem::render(Renderer& renderer) {
    for (auto entity : registry.getEntitiesWith<ParticleComponent, RenderComponent>()) {
        auto& particle = registry.getComponent<ParticleComponent>(entity);
        auto& render = registry.getComponent<RenderComponent>(entity);
        
        // Render the particle using its render component
        renderer.draw(render.texture, particle.position);
    }
}