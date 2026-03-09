#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include <vector>
#include "ecs/Registry.hpp"
#include "components/RenderComponent.hpp"
#include "components/TransformComponent.hpp"
#include "graphics/Renderer.hpp"

class RenderSystem {
public:
    RenderSystem(Renderer& renderer);
    void update(const Registry& registry);

private:
    Renderer& renderer;
    void renderEntity(const TransformComponent& transform, const RenderComponent& render);
};

#endif // RENDER_SYSTEM_HPP