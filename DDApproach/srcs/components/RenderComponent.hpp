#ifndef RENDERCOMPONENT_HPP
#define RENDERCOMPONENT_HPP

#include <string>
#include <vector>
#include <raylib.h>

struct RenderComponent {
    std::string texturePath; // Path to the texture file
    Color color;             // Color tint for the sprite
    Vector2 position;        // Position of the entity in the world
    float rotation;          // Rotation angle of the sprite
    Vector2 scale;           // Scale factor for the sprite

    RenderComponent(const std::string& path, Color col, Vector2 pos, float rot, Vector2 scl)
        : texturePath(path), color(col), position(pos), rotation(rot), scale(scl) {}
};

#endif // RENDERCOMPONENT_HPP