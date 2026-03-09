#pragma once

#include <vector>
#include "RenderComponent.hpp"
#include "Entity.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void render(const std::vector<Entity>& entities);
    void clear();
    void present();

private:
    void drawEntity(const Entity& entity, const RenderComponent& renderComponent);
};