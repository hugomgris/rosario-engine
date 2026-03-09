#ifndef COLLISIONCOMPONENT_HPP
#define COLLISIONCOMPONENT_HPP

#include <vector>
#include <utility>

class CollisionComponent {
public:
    CollisionComponent();
    ~CollisionComponent();

    void addCollider(int entityId, const std::pair<float, float>& size);
    void removeCollider(int entityId);
    const std::vector<std::pair<int, std::pair<float, float>>>& getColliders() const;

private:
    std::vector<std::pair<int, std::pair<float, float>>> colliders; // entityId and size
};

#endif // COLLISIONCOMPONENT_HPP