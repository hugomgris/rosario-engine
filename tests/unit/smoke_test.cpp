#include <gtest/gtest.h>
#include "ecs/Registry.hpp"

namespace {
struct DummyComponent {
    int value = 0;
};
}

TEST(RegistrySmoke, CreateAddGetRemoveComponent) {
    Registry registry;
    Entity entity = registry.createEntity();

    EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));

    registry.addComponent<DummyComponent>(entity, DummyComponent{42});
    ASSERT_TRUE(registry.hasComponent<DummyComponent>(entity));
    EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 42);

    registry.removeComponent<DummyComponent>(entity);
    EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
}
