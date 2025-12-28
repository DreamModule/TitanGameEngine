#pragma once

#include “Entity.hpp”
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <algorithm>
#include <cassert>

namespace Titan::ECS {

class IComponentPool {
public:
virtual ~IComponentPool() = default;
virtual void OnEntityDestroyed(Entity entity) = 0;
virtual size_t Size() const = 0;
virtual bool Has(Entity entity) const = 0;
};

template<typename T>
class ComponentPool : public IComponentPool {
public:
void Add(Entity entity, T&& component) {
assert(!Has(entity));

```
    size_t index = components.size();
    entityToIndex[entity] = index;
    indexToEntity[index] = entity;
    components.push_back(std::forward<T>(component));
}

void Remove(Entity entity) {
    if (!Has(entity)) return;
    
    size_t removedIndex = entityToIndex[entity];
    size_t lastIndex = components.size() - 1;
    
    if (removedIndex != lastIndex) {
        components[removedIndex] = std::move(components[lastIndex]);
        Entity lastEntity = indexToEntity[lastIndex];
        entityToIndex[lastEntity] = removedIndex;
        indexToEntity[removedIndex] = lastEntity;
    }
    
    components.pop_back();
    entityToIndex.erase(entity);
    indexToEntity.erase(lastIndex);
}

T& Get(Entity entity) {
    assert(Has(entity));
    return components[entityToIndex[entity]];
}

const T& Get(Entity entity) const {
    assert(Has(entity));
    return components[entityToIndex.at(entity)];
}

bool Has(Entity entity) const override {
    return entityToIndex.find(entity) != entityToIndex.end();
}

void OnEntityDestroyed(Entity entity) override {
    if (Has(entity)) {
        Remove(entity);
    }
}

size_t Size() const override {
    return components.size();
}

std::vector<T>& GetDense() { return components; }
const std::vector<T>& GetDense() const { return components; }

std::unordered_map<Entity, size_t>& GetSparseMap() { return entityToIndex; }
```

private:
std::vector<T> components;
std::unordered_map<Entity, size_t> entityToIndex;
std::unordered_map<size_t, Entity> indexToEntity;
};

class World {
public:
World() : nextIndex(1) {
generations.reserve(4096);
}

```
~World() {
    Clear();
}

Entity CreateEntity() {
    uint32_t index;
    uint32_t generation;

    if (!freeIndices.empty()) {
        index = freeIndices.front();
        freeIndices.pop();
        generation = generations[index];
    } else {
        index = nextIndex++;
        
        if (index >= generations.size()) {
            generations.resize(index + 1, 0);
        }
        
        generation = 0;
    }

    Entity entity = CreateEntity(index, generation);
    entities.push_back(entity);
    return entity;
}

void DestroyEntity(Entity entity) {
    if (!IsEntityValid(entity)) return;

    uint32_t index = GetEntityIndex(entity);
    uint32_t generation = GetEntityGeneration(entity);

    if (index >= generations.size() || generations[index] != generation) {
        return;
    }

    for (auto& pair : componentPools) {
        pair.second->OnEntityDestroyed(entity);
    }

    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
        entities.erase(it);
    }

    generations[index]++;
    if (generations[index] > ENTITY_GENERATION_MASK) {
        generations[index] = 0;
    }

    freeIndices.push(index);
}

bool IsEntityValid(Entity entity) const {
    if (entity == NULL_ENTITY) return false;

    uint32_t index = GetEntityIndex(entity);
    uint32_t generation = GetEntityGeneration(entity);

    if (index >= generations.size()) return false;

    return generations[index] == generation;
}

template<typename T, typename... Args>
T& AddComponent(Entity entity, Args&&... args) {
    assert(IsEntityValid(entity));
    
    auto pool = GetOrCreatePool<T>();
    pool->Add(entity, T{std::forward<Args>(args)...});
    return pool->Get(entity);
}

template<typename T>
void RemoveComponent(Entity entity) {
    if (!IsEntityValid(entity)) return;
    
    auto pool = GetPool<T>();
    if (pool) {
        pool->Remove(entity);
    }
}

template<typename T>
T& GetComponent(Entity entity) {
    assert(IsEntityValid(entity));
    auto pool = GetPool<T>();
    assert(pool != nullptr);
    return pool->Get(entity);
}

template<typename T>
const T& GetComponent(Entity entity) const {
    assert(IsEntityValid(entity));
    auto pool = GetPool<T>();
    assert(pool != nullptr);
    return pool->Get(entity);
}

template<typename T>
bool HasComponent(Entity entity) const {
    auto pool = GetPool<T>();
    return pool ? pool->Has(entity) : false;
}

template<typename T>
ComponentPool<T>* GetPool() {
    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = componentPools.find(typeIndex);
    if (it == componentPools.end()) return nullptr;
    return static_cast<ComponentPool<T>*>(it->second.get());
}

template<typename T>
const ComponentPool<T>* GetPool() const {
    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = componentPools.find(typeIndex);
    if (it == componentPools.end()) return nullptr;
    return static_cast<const ComponentPool<T>*>(it->second.get());
}

template<typename... Components>
void Each(std::function<void(Entity, Components&...)> func) {
    auto firstPool = GetPool<typename std::tuple_element<0, std::tuple<Components...>>::type>();
    if (!firstPool) return;

    for (auto& [entity, index] : firstPool->GetSparseMap()) {
        if ((HasComponent<Components>(entity) && ...)) {
            func(entity, GetComponent<Components>(entity)...);
        }
    }
}

size_t GetEntityCount() const {
    return entities.size();
}

const std::vector<Entity>& GetEntities() const {
    return entities;
}

void Clear() {
    std::vector<Entity> entitiesToDestroy = entities;
    for (Entity entity : entitiesToDestroy) {
        DestroyEntity(entity);
    }

    entities.clear();
    componentPools.clear();
    
    while (!freeIndices.empty()) {
        freeIndices.pop();
    }
    
    generations.clear();
    nextIndex = 1;
}
```

private:
template<typename T>
ComponentPool<T>* GetOrCreatePool() {
std::type_index typeIndex = std::type_index(typeid(T));
auto it = componentPools.find(typeIndex);

```
    if (it == componentPools.end()) {
        auto pool = std::make_unique<ComponentPool<T>>();
        auto ptr = pool.get();
        componentPools[typeIndex] = std::move(pool);
        return ptr;
    }
    
    return static_cast<ComponentPool<T>*>(it->second.get());
}

std::vector<Entity> entities;
std::queue<uint32_t> freeIndices;
std::vector<uint32_t> generations;
uint32_t nextIndex;

std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> componentPools;
```

};

}
