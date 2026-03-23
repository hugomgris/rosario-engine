# Rosario - Devlog - 6

## Table of Contents
1. [Oopsies](#51---deep-in-portlandia)


<br>
<br>

# 6.1 - Oopsies
I had no plans to log anything related to the test building. That was until one of the first planned tests (from a list of roughly 130) revealed a fundamental error (well, more like a missing step) in the core of the ECS structure, related to entity management in the `Registry`. Therefore, as I predict that this might happen several more times down the line, I'm going to log here the different errors I find. 

The first found issue, as written above, was a missing step in the entity destroy process. `Registry` had a dedicated `destroyEntity()` function, but this was only removing the entity sent as argument from the `_entities` container, without clearing said entity frmo the pool containers. Luckily, the test revealed this issue (I guess it is a good thing I'm building this big ass suite):
```cpp
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
```

The final assertion in this snippet was failing because the registry query was returning a container of size 1. `view()` was polling the pools and finding an entity in the `DummyComponent` one, which was a clear red flag. An easy fix: loop through the component pools and remove the target entity inside `destroyEntity()`:
```cpp
void Registry::destroyEntity(Entity entity) {
    auto it = std::remove(_entities.begin(), _entities.end(), entity);
    _entities.erase(it, _entities.end());

    for (auto& [_, pool] : _componentPools) {
        pool->removeEntity(entity);
    }
}
```