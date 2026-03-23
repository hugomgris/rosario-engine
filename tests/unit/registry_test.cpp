#include <gtest/gtest.h>
#include "ecs/Registry.hpp"

namespace {
	struct DummyComponent {
		int value = 0;
	};

	struct OtherDummyComponent {
		char value = 'a';
	};
}

TEST(RegistryBasic, CreateAddGetRemoveComponent) {
	Registry registry;
	Entity entity = registry.createEntity();

	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));

	registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	ASSERT_TRUE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 42);

	registry.removeComponent<DummyComponent>(entity);
	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
}

TEST(RegistryMultiComponentEntity, BuildEntityWithMultipleComponents) {
	Registry registry;

	Entity entity = registry.createEntity();

	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_FALSE(registry.hasComponent<OtherDummyComponent>(entity));

	registry.addComponent<DummyComponent>(entity, DummyComponent{24});
	registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'x'});

	ASSERT_TRUE(registry.hasComponent<DummyComponent>(entity));
	ASSERT_TRUE(registry.hasComponent<OtherDummyComponent>(entity));

	EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 24);
	EXPECT_EQ(registry.getComponent<OtherDummyComponent>(entity).value, 'x');

	registry.removeComponent<DummyComponent>(entity);
	registry.removeComponent<OtherDummyComponent>(entity);

	ASSERT_FALSE(registry.hasComponent<DummyComponent>(entity));
	ASSERT_FALSE(registry.hasComponent<OtherDummyComponent>(entity));
}

TEST(RegistryComponentPoolIsolation, ComponentsDontAffectEachOther) {
	Registry registry;

	Entity entity = registry.createEntity();

	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_FALSE(registry.hasComponent<OtherDummyComponent>(entity));

	registry.addComponent<DummyComponent>(entity, DummyComponent{24});
	registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'x'});

	registry.removeComponent<OtherDummyComponent>(entity);

	EXPECT_TRUE(registry.hasPool<DummyComponent>());
	EXPECT_TRUE(registry.hasPool<OtherDummyComponent>());

	auto dummyEntities = registry.view<DummyComponent>();
	EXPECT_EQ(dummyEntities.size(), 1);

	auto otherDummyEntities = registry.view<OtherDummyComponent>();
	EXPECT_EQ(otherDummyEntities.size(), 0);
}

TEST(EntityDestruction, NoDanglingReferences) {
	Registry registry;

	Entity entity = registry.createEntity();

	registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'a'});

	registry.destroyEntity(entity);

	EXPECT_TRUE(registry.hasPool<DummyComponent>());
	EXPECT_TRUE(registry.hasPool<OtherDummyComponent>());

	auto entities = registry.view<DummyComponent>();
	EXPECT_EQ(entities.size(), 0);
}
