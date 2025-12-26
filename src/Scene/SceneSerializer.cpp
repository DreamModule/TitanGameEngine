#include "SceneSerializer.hpp"
#include "../ECS/Components/Transform.hpp"
#include "../ECS/Components/Sprite.hpp"
#include <fstream>
#include <sstream>

namespace Titan::Scene {

bool SceneSerializer::SaveScene(Scene* scene, const std::string& path) {
    if (!scene || !scene->world) return false;
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f.is_open()) return false;
    f << "Scene " << scene->name << "\n";
    for (auto const& p : scene->world->transforms) {
        auto e = p.first;
        auto &t = p.second;
        f << "Transform " << e << " " << t.x << " " << t.y << " " << t.sx << " " << t.sy << " " << t.rot << "\n";
    }
    for (auto const& p : scene->world->sprites) {
        auto e = p.first;
        auto &s = p.second;
        f << "Sprite " << e << " " << s.path << "\n";
    }
    f.close();
    return true;
}

bool SceneSerializer::LoadScene(Scene* scene, const std::string& path) {
    if (!scene) return false;
    std::ifstream f(path);
    if (!f.is_open()) return false;
    scene->world = std::make_unique<Titan::ECS::World>();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "Scene") {
            std::string name;
            iss >> name;
            scene->name = name;
        } else if (token == "Transform") {
            Titan::ECS::Entity e;
            Titan::ECS::Components::Transform t;
            iss >> e >> t.x >> t.y >> t.sx >> t.sy >> t.rot;
            scene->world->transforms.emplace(e, t);
            if (e >= scene->world->nextId) scene->world->nextId = e + 1;
        } else if (token == "Sprite") {
            Titan::ECS::Entity e;
            std::string pathTok;
            iss >> e >> pathTok;
            Titan::ECS::Components::Sprite s;
            s.path = pathTok;
            scene->world->sprites.emplace(e, s);
            if (e >= scene->world->nextId) scene->world->nextId = e + 1;
        }
    }
    f.close();
    return true;
}

}
