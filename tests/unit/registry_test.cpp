#include <gtest/gtest.h>
#include <utility>
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

	struct TrackingComponent {
		int value = 0;

		inline static int copyCtorCount = 0;
		inline static int moveCtorCount = 0;
		inline static int copyAssignCount = 0;
		inline static int moveAssignCount = 0;

		explicit TrackingComponent(int v = 0) : value(v) {}

		TrackingComponent(const TrackingComponent& other) : value(other.value) {
			++copyCtorCount;
		}

		TrackingComponent(TrackingComponent&& other) noexcept : value(other.value) {
			++moveCtorCount;
			other.value = -1;
		}

		TrackingComponent& operator=(const TrackingComponent& other) {
			if (this != &other)
				value = other.value;
			++copyAssignCount;
			return *this;
		}

		TrackingComponent& operator=(TrackingComponent&& other) noexcept {
			if (this != &other) {
				value = other.value;
				other.value = -1;
			}
			++moveAssignCount;
			return *this;
		}

		static void resetCounters() {
			copyCtorCount = 0;
			moveCtorCount = 0;
			copyAssignCount = 0;
			moveAssignCount = 0;
		}
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
TEST(Registry, ShouldStoreMultipleComponentTypes_OnSameEntity) {
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
TEST(Registry, ShouldKeepComponentPoolsIsolated_WhenRemovingDifferentType) {
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
TEST(Registry, ShouldRemoveAllComponents_WhenEntityIsDestroyed) {
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
TEST(Registry, ShouldReturnFalseFromHasComponent_AfterComponentRemoval) {
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
TEST(Registry, ShouldMutateOnlyMatchingComponentType_WhenUsingForEachSingleType) {
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
TEST(Registry, ShouldReturnOnlyEntitiesMatchingAllFilters_WhenUsingViewWithMultipleTypes) {
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
TEST(Registry, ShouldThrowRuntimeError_WhenGettingNonExistentComponent) {
	Registry registry;

	Entity entity = registry.createEntity();
	registry.addComponent<DummyComponent>(entity, DummyComponent{42});

	EXPECT_ANY_THROW(registry.getComponent<OtherDummyComponent>(entity));
	EXPECT_THROW(registry.getComponent<OtherDummyComponent>(entity), std::runtime_error);
}

// 9 - Registry: Accessing destroyed entity throws exception
TEST(Registry, ShouldThrowRuntimeError_WhenGettingComponentFromDestroyedEntity) {
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

	EXPECT_ANY_THROW(registry.getComponent<DummyComponent>(entity));
	EXPECT_THROW(registry.getComponent<DummyComponent>(entity), std::runtime_error);
}

// 10 - ComponentPool: Capacity growth under stress (many entities)
TEST(Registry, ShouldHandleHighEntityVolumeWithoutCrash_WhenGrowingComponentPools) {
	Registry registry;

	for (int i = 0; i < 1000; i++) {
		Entity entity = registry.createEntity();
		registry.addComponent<DummyComponent>(entity, DummyComponent{42});
	}

	EXPECT_EQ(registry.getEntities().size(), 1000);

	auto entities = registry.view<DummyComponent>();
	EXPECT_EQ(entities.size(), 1000);

	for (int i = 0; i < 2000; i++) {
		Entity entity = registry.createEntity();
		registry.addComponent<OtherDummyComponent>(entity, OtherDummyComponent{'a'});
		if (i % 2 == 0) {
			registry.addComponent<ForEachComponentOne>(entity, ForEachComponentOne{42});
		}
	}

	EXPECT_EQ(registry.getEntities().size(), 3000);
	auto entities2 = registry.view<OtherDummyComponent>();
	EXPECT_EQ(entities2.size(), 2000);
	
	auto entities3 = registry.view<ForEachComponentOne>();
	EXPECT_EQ(entities3.size(), 1000);
}

// 11 - ComponentPool: Copy and move semantics for components
TEST(Registry, ShouldSupportCopyAndMoveInsertion_WhenAddingComponents) {
	TrackingComponent::resetCounters();
	Registry registry;

	Entity copyEntity = registry.createEntity();
	Entity moveEntity = registry.createEntity();

	TrackingComponent copySource(7);
	registry.addComponent<TrackingComponent>(copyEntity, copySource);

	EXPECT_GE(TrackingComponent::copyCtorCount, 1);
	EXPECT_EQ(registry.getComponent<TrackingComponent>(copyEntity).value, 7);

	TrackingComponent moveSource(13);
	registry.addComponent<TrackingComponent>(moveEntity, std::move(moveSource));

	EXPECT_GE(TrackingComponent::moveCtorCount, 1);
	EXPECT_EQ(registry.getComponent<TrackingComponent>(moveEntity).value, 13);

	// Registry still stores independent component state after copy/move insertions.
	registry.getComponent<TrackingComponent>(copyEntity).value = 21;
	EXPECT_EQ(registry.getComponent<TrackingComponent>(copyEntity).value, 21);
	EXPECT_EQ(registry.getComponent<TrackingComponent>(moveEntity).value, 13);
}