#include “Engine.hpp”
#include <chrono>
#include <thread>
#include <atomic>
#include <stdexcept>

namespace Titan {
namespace Engine {

namespace {
struct EngineState {
Config config;
Stats stats;

```
    std::atomic<bool> running{false};
    std::atomic<bool> initialized{false};
    
    std::function<void(float)> fixedUpdateCallback;
    std::function<void(float)> updateCallback;
    std::function<void()> renderCallback;
    
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float accumulator = 0.0f;
};

EngineState* g_state = nullptr;

void UpdateStats(float deltaTime) {
    auto& stats = g_state->stats;
    stats.deltaTime = deltaTime;
    stats.frameTime = deltaTime * 1000.0f;
    stats.frameCount++;
    
    if (deltaTime > 0.0f) {
        stats.fps = 1.0f / deltaTime;
    }
}

void LimitFramerate(const std::chrono::high_resolution_clock::time_point& frameStart) {
    if (g_state->config.maxFPS == 0) return;
    
    auto targetFrameTime = std::chrono::microseconds(1000000 / g_state->config.maxFPS);
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto frameDuration = frameEnd - frameStart;
    
    if (frameDuration < targetFrameTime) {
        std::this_thread::sleep_for(targetFrameTime - frameDuration);
    }
}
```

}

void Init(const Config& config) {
if (g_state != nullptr) {
throw std::runtime_error(“Engine already initialized”);
}

```
g_state = new EngineState();
g_state->config = config;
g_state->initialized.store(true);
g_state->running.store(false);
g_state->lastFrameTime = std::chrono::high_resolution_clock::now();
```

}

void Run(std::function<void(float)> updateCallback) {
if (!g_state || !g_state->initialized.load()) {
throw std::runtime_error(“Engine not initialized”);
}

```
if (g_state->running.exchange(true)) {
    throw std::runtime_error("Engine already running");
}

g_state->updateCallback = updateCallback;
g_state->lastFrameTime = std::chrono::high_resolution_clock::now();
g_state->accumulator = 0.0f;

while (g_state->running.load()) {
    auto frameStart = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<float> elapsed = frameStart - g_state->lastFrameTime;
    float deltaTime = elapsed.count();
    g_state->lastFrameTime = frameStart;
    
    if (deltaTime > 0.25f) {
        deltaTime = 0.25f;
    }
    
    UpdateStats(deltaTime);
    
    g_state->accumulator += deltaTime;
    
    uint32_t fixedSteps = 0;
    while (g_state->accumulator >= g_state->config.fixedTimestep && 
           fixedSteps < g_state->config.maxFixedStepsPerFrame) {
        
        if (g_state->fixedUpdateCallback) {
            g_state->fixedUpdateCallback(g_state->config.fixedTimestep);
        }
        
        g_state->accumulator -= g_state->config.fixedTimestep;
        fixedSteps++;
    }
    
    if (g_state->updateCallback) {
        g_state->updateCallback(deltaTime);
    }
    
    if (g_state->renderCallback) {
        g_state->renderCallback();
    }
    
    LimitFramerate(frameStart);
}
```

}

void Shutdown() {
if (!g_state) return;

```
g_state->running.store(false);
g_state->initialized.store(false);

delete g_state;
g_state = nullptr;
```

}

bool IsRunning() {
return g_state && g_state->running.load();
}

void RequestQuit() {
if (g_state) {
g_state->running.store(false);
}
}

const Config& GetConfig() {
if (!g_state) {
throw std::runtime_error(“Engine not initialized”);
}
return g_state->config;
}

const Stats& GetStats() {
if (!g_state) {
throw std::runtime_error(“Engine not initialized”);
}
return g_state->stats;
}

void SetFixedUpdateCallback(std::function<void(float)> callback) {
if (g_state) {
g_state->fixedUpdateCallback = callback;
}
}

void SetRenderCallback(std::function<void()> callback) {
if (g_state) {
g_state->renderCallback = callback;
}
}

}
}
