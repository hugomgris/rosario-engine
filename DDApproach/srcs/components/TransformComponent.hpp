#ifndef TRANSFORMCOMPONENT_HPP
#define TRANSFORMCOMPONENT_HPP

#include <glm/glm.hpp>

class TransformComponent {
public:
    glm::vec3 position; // Position of the entity in 3D space
    glm::vec3 rotation; // Rotation of the entity in Euler angles
    glm::vec3 scale;    // Scale of the entity

    TransformComponent(const glm::vec3& pos = glm::vec3(0.0f), 
                       const glm::vec3& rot = glm::vec3(0.0f), 
                       const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {}
};

#endif // TRANSFORMCOMPONENT_HPP