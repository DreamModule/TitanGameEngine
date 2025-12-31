#include "SceneManager.hpp"

namespace Titan::Scene {

SceneManager::SceneManager() {}

Scene* SceneManager::CreateScene(const std::string& name) {
    if (scenes.find(name) != scenes.end()) return scenes[name].get();
    auto s = std::make_unique<Scene>();
    s->name = name;
    s->world = std::make_unique<Titan::ECS::World>();
    Scene* ptr = s.get();
    scenes.emplace(name, std::move(s));
    return ptr;
}

Scene* SceneManager::GetScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it == scenes.end()) return nullptr;
    return it->second.get();
}

bool SceneManager::ActivateScene(const std::string& name) {
    Scene* s = GetScene(name);
    if (!s) return false;
    active = s;
    return true;
}

Scene* SceneManager::GetActiveScene() {
    return active;
}

bool SceneManager::DestroyScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it == scenes.end()) return false;
    if (active == it->second.get()) active = nullptr;
    scenes.erase(it);
    return true;
}

}
