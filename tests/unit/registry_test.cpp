#include <gtest/gtest.h>
#include "ecs/Registry.hpp"

namespace {
	struct DummyComponent {
		int value = 0;
	};

	struct OtherDummyComponent {
		char value = 'a';
	};

	struct ForEachComponentOne {
		int value = 2;
	};

	struct ForEachComponentTwo {
		int value = 3;
	};
}

// 1 - Entity basic creation and handling
TEST(Registry, ShouldAddGetAndRemoveComponent_WhenSingleComponentLifecycle) {
	Registry registry;
	Entity entity = registry.createEntity();

	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));

	registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	ASSERT_TRUE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 42);

	registry.removeComponent<DummyComponent>(entity);
	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
}

// 2 - Registry: Multiple component types on single entity
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

// 3 - Registry: Component pool isolation (one component type doesn't affect another)
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

// 4 - Registry: Entity destruction and cleanup (no dangling references)
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
	auto entities2 = registry.view<OtherDummyComponent>();
	EXPECT_EQ(entities2.size(), 0);
}

// 5 - Registry: hasComponent() returns false after removeComponent()
TEST(RemoveComponent, HasComponentReturnsFalse) {
	Registry registry;

	Entity entity = registry.createEntity();

	registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'a'});

	EXPECT_TRUE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_TRUE(registry.hasComponent<OtherDummyComponent>(entity));

	registry.removeComponent<DummyComponent>(entity);
	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_TRUE(registry.hasComponent<OtherDummyComponent>(entity));

	registry.removeComponent<OtherDummyComponent>(entity);
	EXPECT_FALSE(registry.hasComponent<DummyComponent>(entity));
	EXPECT_FALSE(registry.hasComponent<OtherDummyComponent>(entity));
}

// 6 - Registry: forEach() with single component type
TEST(ForEachComponent, ComponentCompartimentalization) {
	Registry registry;

	Entity entity = registry.createEntity();
	Entity entity2 = registry.createEntity();
	Entity entity3 = registry.createEntity();

	registry.addComponent<ForEachComponentOne>(entity, ForEachComponentOne{2});
	registry.addComponent<ForEachComponentTwo>(entity, ForEachComponentTwo{3});
	registry.addComponent<ForEachComponentOne>(entity2, ForEachComponentOne{3});
	registry.addComponent<ForEachComponentTwo>(entity3, ForEachComponentTwo{4});

	EXPECT_TRUE(registry.hasComponent<ForEachComponentOne>(entity));
	EXPECT_TRUE(registry.hasComponent<ForEachComponentTwo>(entity));
	EXPECT_TRUE(registry.hasComponent<ForEachComponentOne>(entity2));
	EXPECT_TRUE(registry.hasComponent<ForEachComponentTwo>(entity3));

	EXPECT_EQ(registry.getComponent<ForEachComponentOne>(entity).value, 2);

	registry.forEach<ForEachComponentOne>([](Entity entity, ForEachComponentOne& component) {
		component.value *= 10;
	} );

	// ForEach should have affected every entity with a ForEachComponentOne component

	EXPECT_EQ(registry.getComponent<ForEachComponentOne>(entity).value, 20);
	EXPECT_EQ(registry.getComponent<ForEachComponentTwo>(entity).value, 3);
	EXPECT_EQ(registry.getComponent<ForEachComponentOne>(entity2).value, 30);
	EXPECT_TRUE(registry.hasComponent<ForEachComponentOne>(entity3) == false);
}

// 7 - Registry: view() with multiple component filters
TEST(RegistryView, ShouldReturnOnlyEntitiesMatchingAllFilters_WhenUsingViewWithMultipleTypes) {
	Registry registry;

	Entity entity = registry.createEntity();
	Entity entity2 = registry.createEntity();
	Entity entity3 = registry.createEntity();
	Entity entity4 = registry.createEntity();

	// entity 1 has 1,2,3,4 components
	registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'a'});
	registry.addComponent<ForEachComponentOne>(entity, ForEachComponentOne{42});
	registry.addComponent<ForEachComponentTwo>(entity, ForEachComponentTwo{42});

	// entity 2 has 1,3 components
	registry.addComponent<DummyComponent>(entity2, DummyComponent{42});
	registry.addComponent<ForEachComponentOne>(entity2, ForEachComponentOne{42});

	// entity 3 has 2,4 components
	registry.addComponent<OtherDummyComponent>(entity3, OtherDummyComponent{'a'});
	registry.addComponent<ForEachComponentTwo>(entity3, ForEachComponentTwo{42});

	// entity 4 has 1,4 components
	registry.addComponent<DummyComponent>(entity4, DummyComponent{42});
	registry.addComponent<ForEachComponentTwo>(entity4, ForEachComponentTwo{42});

	// should be 2 entities with components 1,3
	auto e1 = registry.view<DummyComponent, ForEachComponentOne>();
	EXPECT_EQ(e1.size(), 2);

	// should be 2 entities with components 1,4
	auto e2 = registry.view<DummyComponent, ForEachComponentTwo>();
	EXPECT_EQ(e2.size(), 2);

	// should be 3 entities with component 4
	auto e3 = registry.view<ForEachComponentTwo>();
	EXPECT_EQ(e3.size(), 3);

	registry.addComponent<ForEachComponentTwo>(entity2, ForEachComponentTwo{42});

	// now 4
	auto e4 = registry.view<ForEachComponentTwo>();
	EXPECT_EQ(e4.size(), 4);

	registry.removeComponent<ForEachComponentTwo>(entity);
	registry.removeComponent<ForEachComponentTwo>(entity4);

	// now 2
	auto e5 = registry.view<ForEachComponentTwo>();
	EXPECT_EQ(e5.size(), 2);

	registry.addComponent<OtherDummyComponent>(entity2, OtherDummyComponent{'b'});
	registry.addComponent<OtherDummyComponent>(entity4, OtherDummyComponent{'c'});

	// Quering for OtherDummyComponent and ForEachComponentOne should now return 3 entities
	auto e6 = registry.view<DummyComponent, ForEachComponentOne>();
	EXPECT_EQ(e6.size(), 2);
}

// 8 - Registry: Attempting to get non-existent component throws exception
TEST(NonExistingComponentFetching, ShouldThrow) {
	Registry registry;

	Entity entity = registry.createEntity();
	registry.addComponent<DummyComponent>(entity, DummyComponent{42});

	EXPECT_ANY_THROW(registry.getComponent<OtherDummyComponent>(entity));
	EXPECT_THROW(registry.getComponent<OtherDummyComponent>(entity), std::runtime_error);
}

