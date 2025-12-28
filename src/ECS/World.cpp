#include “World.hpp”

namespace Titan {
namespace ECS {

World::World() : nextIndex(1) {
generations.reserve(1000);
}

World::~World() {
Clear();
}

Entity World::CreateEntity() {
uint32_t index;
uint32_t generation;

```
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
```

}

void World::DestroyEntity(Entity entity) {
if (!IsEntityValid(entity)) return;

```
uint32_t index = GetEntityIndex(entity);
uint32_t generation = GetEntityGeneration(entity);

if (index >= generations.size() || generations[index] != generation) {
    return;
}

componentManager.OnEntityDestroyed(entity);

auto it = std::find(entities.begin(), entities.end(), entity);
if (it != entities.end()) {
    entities.erase(it);
}

generations[index]++;
if (generations[index] > ENTITY_GENERATION_MASK) {
    generations[index] = 0;
}

freeIndices.push(index);
```

}

bool World::IsEntityValid(Entity entity) const {
if (entity == NULL_ENTITY) return false;

```
uint32_t index = GetEntityIndex(entity);
uint32_t generation = GetEntityGeneration(entity);

if (index >= generations.size()) return false;

return generations[index] == generation;
```

}

void World::RegisterSystem(std::shared_ptr<ISystem> system, int priority) {
if (!system) return;

```
SystemEntry entry;
entry.system = system;
entry.priority = priority;

systems.push_back(entry);
SortSystems();

system->OnInit(this);
```

}

void World::UnregisterSystem(std::shared_ptr<ISystem> system) {
if (!system) return;

```
auto it = std::find_if(systems.begin(), systems.end(),
    [&system](const SystemEntry& entry) {
        return entry.system == system;
    });

if (it != systems.end()) {
    it->system->OnShutdown();
    systems.erase(it);
}
```

}

void World::Update(float deltaTime) {
for (auto& entry : systems) {
if (entry.system) {
entry.system->OnUpdate(deltaTime);
}
}
}

void World::FixedUpdate(float fixedDeltaTime) {
for (auto& entry : systems) {
if (entry.system) {
entry.system->OnFixedUpdate(fixedDeltaTime);
}
}
}

void World::Render() {
for (auto& entry : systems) {
if (entry.system) {
entry.system->OnRender();
}
}
}

size_t World::GetEntityCount() const {
return entities.size();
}

const std::vector<Entity>& World::GetEntities() const {
return entities;
}

void World::Clear() {
for (auto& entry : systems) {
if (entry.system) {
entry.system->OnShutdown();
}
}
systems.clear();

```
std::vector<Entity> entitiesToDestroy = entities;
for (Entity entity : entitiesToDestroy) {
    DestroyEntity(entity);
}

entities.clear();
while (!freeIndices.empty()) {
    freeIndices.pop();
}
generations.clear();
nextIndex = 1;
```

}

void World::SortSystems() {
std::sort(systems.begin(), systems.end());
}

}
}
