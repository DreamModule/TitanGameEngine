#pragma once

#include “../Scene/SceneManager.hpp”
#include “PhysicsWorld.hpp”

namespace Titan::Physics {

class PhysicsSystem : public Scene::ISystem {
public:
PhysicsSystem() {
PhysicsSettings settings;
settings.gravity = Vec3(0, -9.81f, 0);
settings.fixedTimestep = 1.0f / 60.0f;
settings.solverIterations = 8;

```
    PhysicsWorld::Get().Init(settings);
}

void OnInit() override {
    auto scene = Scene::SceneManager::Get().GetActiveScene();
    if (scene && scene->world) {
        PhysicsWorld::Get().SetWorld(scene->world.get());
    }
}

void OnFixedUpdate(float fixedDt) override {
    PhysicsWorld::Get().FixedStep(fixedDt);
}

void OnShutdown() override {
    PhysicsWorld::Get().Shutdown();
}
```

};

}
