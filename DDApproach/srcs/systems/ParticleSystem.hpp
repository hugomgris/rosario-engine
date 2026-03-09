#ifndef PARTICLESYSTEM_HPP
#define PARTICLESYSTEM_HPP

#include <vector>
#include "ecs/Entity.hpp"
#include "components/TransformComponent.hpp"
#include "components/RenderComponent.hpp"

class Particle {
public:
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float age;
    Color color;

    Particle(const Vector2& pos, const Vector2& vel, float life, const Color& col)
        : position(pos), velocity(vel), lifetime(life), age(0.0f), color(col) {}
};

class ParticleSystem {
public:
    ParticleSystem();
    void update(float deltaTime);
    void render(const Renderer& renderer);
    void emit(const Vector2& position, const Vector2& velocity, float lifetime, const Color& color);
    void clear();

private:
    std::vector<Particle> particles;
};

#endif // PARTICLESYSTEM_HPP