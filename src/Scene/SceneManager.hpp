#pragma once
#include "../ECS/World.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace Titan::Scene {

struct Scene {
    std::string name;
    std::unique_ptr<Titan::ECS::World> world;
};

struct SceneManager {
    SceneManager();
    Scene* CreateScene(const std::string& name);
    Scene* GetScene(const std::string& name);
    bool ActivateScene(const std::string& name);
    Scene* GetActiveScene();
    bool DestroyScene(const std::string& name);
    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* active = nullptr;
};
}
