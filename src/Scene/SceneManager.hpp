#pragma once

#include “../ECS/World.hpp”
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Titan::Scene {

class ISystem {
public:
virtual ~ISystem() = default;
virtual void OnInit() {}
virtual void OnShutdown() {}
virtual void OnUpdate(float dt) {}
virtual void OnFixedUpdate(float fixedDt) {}
virtual void OnRender() {}
};

struct Scene {
std::string name;
std::unique_ptr<ECS::World> world;

```
struct SystemEntry {
    std::shared_ptr<ISystem> system;
    int priority;
    
    bool operator<(const SystemEntry& other) const {
        return priority < other.priority;
    }
};

std::vector<SystemEntry> systems;

Scene(const std::string& name) : name(name) {
    world = std::make_unique<ECS::World>();
}

void RegisterSystem(std::shared_ptr<ISystem> system, int priority = 0) {
    SystemEntry entry;
    entry.system = system;
    entry.priority = priority;
    
    systems.push_back(entry);
    std::sort(systems.begin(), systems.end());
    
    system->OnInit();
}

void Update(float dt) {
    for (auto& entry : systems) {
        if (entry.system) {
            entry.system->OnUpdate(dt);
        }
    }
}

void FixedUpdate(float fixedDt) {
    for (auto& entry : systems) {
        if (entry.system) {
            entry.system->OnFixedUpdate(fixedDt);
        }
    }
}

void Render() {
    for (auto& entry : systems) {
        if (entry.system) {
            entry.system->OnRender();
        }
    }
}

~Scene() {
    for (auto& entry : systems) {
        if (entry.system) {
            entry.system->OnShutdown();
        }
    }
}
```

};

class SceneManager {
public:
static SceneManager& Get() {
static SceneManager instance;
return instance;
}

```
Scene* CreateScene(const std::string& name) {
    if (scenes.find(name) != scenes.end()) {
        return scenes[name].get();
    }
    
    auto scene = std::make_unique<Scene>(name);
    Scene* ptr = scene.get();
    scenes.emplace(name, std::move(scene));
    
    return ptr;
}

Scene* GetScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it == scenes.end()) return nullptr;
    return it->second.get();
}

bool ActivateScene(const std::string& name) {
    Scene* scene = GetScene(name);
    if (!scene) return false;
    
    active = scene;
    return true;
}

Scene* GetActiveScene() {
    return active;
}

bool DestroyScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it == scenes.end()) return false;
    
    if (active == it->second.get()) {
        active = nullptr;
    }
    
    scenes.erase(it);
    return true;
}

void UpdateActive(float dt) {
    if (active) {
        active->Update(dt);
    }
}

void FixedUpdateActive(float fixedDt) {
    if (active) {
        active->FixedUpdate(fixedDt);
    }
}

void RenderActive() {
    if (active) {
        active->Render();
    }
}
```

private:
SceneManager() = default;
~SceneManager() = default;

```
SceneManager(const SceneManager&) = delete;
SceneManager& operator=(const SceneManager&) = delete;

std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
Scene* active = nullptr;
```

};

}
