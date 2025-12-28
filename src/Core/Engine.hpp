#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace Titan {
namespace Engine {

struct Config {
uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;
std::string windowTitle = “Titan Engine”;
bool fullscreen = false;
bool vsync = true;
uint32_t maxFPS = 0;

```
float fixedTimestep = 1.0f / 60.0f;
uint32_t maxFixedStepsPerFrame = 5;

bool enablePhysics = true;
bool enableAudio = true;

void* nativeWindowHandle = nullptr;
```

};

struct Stats {
float fps = 0.0f;
float frameTime = 0.0f;
float deltaTime = 0.0f;
uint64_t frameCount = 0;
uint32_t drawCalls = 0;
uint32_t triangles = 0;
};

void Init(const Config& config);
void Run(std::function<void(float)> updateCallback);
void Shutdown();

bool IsRunning();
void RequestQuit();

const Config& GetConfig();
const Stats& GetStats();

void SetFixedUpdateCallback(std::function<void(float)> callback);
void SetRenderCallback(std::function<void()> callback);

}
}
