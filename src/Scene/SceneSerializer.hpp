#pragma once
#include "../Scene/SceneManager.hpp"
#include <string>

namespace Titan::Scene {

struct SceneSerializer {
    static bool SaveScene(Scene* scene, const std::string& path);
    static bool LoadScene(Scene* scene, const std::string& path);
};

}
