#pragma once

#include “Entity.hpp”
#include “ComponentManager.hpp”
#include “System.hpp”
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>

namespace Titan {
namespace ECS {

class World {
public:
World();
~World();

```
Entity CreateEntity();
void DestroyEntity(Entity entity);
bool IsEntityValid(Entity entity) const;

template<typename T>
void RegisterComponent() {
    componentManager.RegisterComponent<T>();
}

template<typename T>
void AddComponent(Entity entity, const T& component) {
    if (!IsEntityValid(entity)) return;
    componentManager.AddComponent<T>(entity, component);
}

template<typename T>
void RemoveComponent(Entity entity) {
    if (!IsEntityValid(entity)) return;
    componentManager.RemoveComponent<T>(entity);
}

template<typename T>
T& GetComponent(Entity entity) {
    return componentManager.GetComponent<T>(entity);
}

template<typename T>
const T& GetComponent(Entity entity) const {
    return componentManager.GetComponent<T>(entity);
}

template<typename T>
bool HasComponent(Entity entity) const {
    return componentManager.HasComponent<T>(entity);
}

template<typename T>
std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    return componentManager.GetComponentArray<T>();
}

template<typename T>
std::shared_ptr<const ComponentArray<T>> GetComponentArray() const {
    return componentManager.GetComponentArray<T>();
}

void RegisterSystem(std::shared_ptr<ISystem> system, int priority = 0);
void UnregisterSystem(std::shared_ptr<ISystem> system);

void Update(float deltaTime);
void FixedUpdate(float fixedDeltaTime);
void Render();

size_t GetEntityCount() const;
const std::vector<Entity>& GetEntities() const;

void Clear();
```

private:
ComponentManager componentManager;

```
std::vector<Entity> entities;
std::queue<uint32_t> freeIndices;
std::vector<uint32_t> generations;

struct SystemEntry {
    std::shared_ptr<ISystem> system;
    int priority;
    
    bool operator<(const SystemEntry& other) const {
        return priority < other.priority;
    }
};

std::vector<SystemEntry> systems;

uint32_t nextIndex;

void SortSystems();
```

};

}
}
