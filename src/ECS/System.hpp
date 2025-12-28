#pragma once

namespace Titan {
namespace ECS {

class World;

class ISystem {
public:
virtual ~ISystem() = default;

```
virtual void OnInit(World* world) { 
    this->world = world;
}

virtual void OnShutdown() {}

virtual void OnUpdate(float deltaTime) {}

virtual void OnFixedUpdate(float fixedDeltaTime) {}

virtual void OnRender() {}
```

protected:
World* world = nullptr;
};

}
}
