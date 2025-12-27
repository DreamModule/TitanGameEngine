#include "RenderEntitySystem.hpp"
#include "../../Scene/SceneManager.hpp"

static Titan::Scene::SceneManager* s_mgr = nullptr;

extern "C" void RenderEntitySystem_SetSceneManager(Titan::Scene::SceneManager* m) {
    s_mgr = m;
}

namespace {
    // internal access for system file to get pointer
}

#endif
