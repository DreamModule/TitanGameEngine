#pragma once

namespace Titan::Core {

struct EngineContext {
    bool running = true;
    float deltaTime = 0.0f;
};

namespace Engine {
    bool Init(EngineContext& ctx);
    void Update(EngineContext& ctx);
    void Shutdown(EngineContext& ctx);
}

}
