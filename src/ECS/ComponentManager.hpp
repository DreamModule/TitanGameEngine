#pragma once

#include “Entity.hpp”
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <vector>
#include <algorithm>

namespace Titan {
namespace ECS {

class IComponentArray {
public:
virtual ~IComponentArray() = default;
virtual void OnEntityDestroyed(Entity entity) = 0;
virtual size_t GetSize() const = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
void Add(Entity entity, const T& component) {
if (entityToIndex.find(entity) != entityToIndex.end()) {
return;
}

```
    size_t newIndex = components.size();
    entityToIndex[entity] = newIndex;
    indexToEntity[newIndex] = entity;
    components.push_back(component);
}

void Remove(Entity entity) {
    if (entityToIndex.find(entity) == entityToIndex.end()) {
        return;
    }
    
    size_t indexOfRemoved = entityToIndex[entity];
    size_t indexOfLast = components.size() - 1;
    
    if (indexOfRemoved != indexOfLast) {
        components[indexOfRemoved] = components[indexOfLast];
        
        Entity entityOfLast = indexToEntity[indexOfLast];
        entityToIndex[entityOfLast] = indexOfRemoved;
        indexToEntity[indexOfRemoved] = entityOfLast;
    }
    
    components.pop_back();
    entityToIndex.erase(entity);
    indexToEntity.erase(indexOfLast);
}

T& Get(Entity entity) {
    return components[entityToIndex[entity]];
}

const T& Get(Entity entity) const {
    return components[entityToIndex.at(entity)];
}

bool Has(Entity entity) const {
    return entityToIndex.find(entity) != entityToIndex.end();
}

void OnEntityDestroyed(Entity entity) override {
    if (Has(entity)) {
        Remove(entity);
    }
}

size_t GetSize() const override {
    return components.size();
}

std::vector<T>& GetComponents() {
    return components;
}

const std::vector<T>& GetComponents() const {
    return components;
}

std::unordered_map<Entity, size_t>& GetEntityToIndexMap() {
    return entityToIndex;
}
```

private:
std::vector<T> components;
std::unordered_map<Entity, size_t> entityToIndex;
std::unordered_map<size_t, Entity> indexToEntity;
};

class ComponentManager {
public:
template<typename T>
void RegisterComponent() {
std::type_index typeIndex = std::type_index(typeid(T));

```
    if (componentArrays.find(typeIndex) != componentArrays.end()) {
        return;
    }
    
    componentArrays[typeIndex] = std::make_shared<ComponentArray<T>>();
}

template<typename T>
void AddComponent(Entity entity, const T& component) {
    GetComponentArray<T>()->Add(entity, component);
}

template<typename T>
void RemoveComponent(Entity entity) {
    GetComponentArray<T>()->Remove(entity);
}

template<typename T>
T& GetComponent(Entity entity) {
    return GetComponentArray<T>()->Get(entity);
}

template<typename T>
const T& GetComponent(Entity entity) const {
    return GetComponentArray<T>()->Get(entity);
}

template<typename T>
bool HasComponent(Entity entity) const {
    auto array = GetComponentArray<T>();
    return array ? array->Has(entity) : false;
}

void OnEntityDestroyed(Entity entity) {
    for (auto& pair : componentArrays) {
        pair.second->OnEntityDestroyed(entity);
    }
}

template<typename T>
std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    std::type_index typeIndex = std::type_index(typeid(T));
    
    if (componentArrays.find(typeIndex) == componentArrays.end()) {
        RegisterComponent<T>();
    }
    
    return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeIndex]);
}

template<typename T>
std::shared_ptr<const ComponentArray<T>> GetComponentArray() const {
    std::type_index typeIndex = std::type_index(typeid(T));
    
    auto it = componentArrays.find(typeIndex);
    if (it == componentArrays.end()) {
        return nullptr;
    }
    
    return std::static_pointer_cast<const ComponentArray<T>>(it->second);
}
```

private:
std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays;
};

}
}
