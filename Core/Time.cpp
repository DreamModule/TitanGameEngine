#include “Time.hpp”
#include <chrono>

namespace Titan {
namespace Time {

namespace {
struct TimeState {
float deltaTime = 0.0f;
float deltaTimeUnscaled = 0.0f;
float totalTime = 0.0f;
float fixedDeltaTime = 1.0f / 60.0f;
float timeScale = 1.0f;
uint64_t frameCount = 0;
std::chrono::high_resolution_clock::time_point startTime;
};

```
TimeState* g_timeState = nullptr;
```

}

void Init() {
if (g_timeState == nullptr) {
g_timeState = new TimeState();
g_timeState->startTime = std::chrono::high_resolution_clock::now();
}
}

void Update(float deltaTime) {
if (!g_timeState) return;

```
g_timeState->deltaTimeUnscaled = deltaTime;
g_timeState->deltaTime = deltaTime * g_timeState->timeScale;
g_timeState->totalTime += g_timeState->deltaTime;
g_timeState->frameCount++;
```

}

float Delta() {
return g_timeState ? g_timeState->deltaTime : 0.0f;
}

float DeltaUnscaled() {
return g_timeState ? g_timeState->deltaTimeUnscaled : 0.0f;
}

float Total() {
return g_timeState ? g_timeState->totalTime : 0.0f;
}

float Fixed() {
return g_timeState ? g_timeState->fixedDeltaTime : (1.0f / 60.0f);
}

void SetTimeScale(float scale) {
if (g_timeState && scale >= 0.0f) {
g_timeState->timeScale = scale;
}
}

float GetTimeScale() {
return g_timeState ? g_timeState->timeScale : 1.0f;
}

uint64_t GetFrameCount() {
return g_timeState ? g_timeState->frameCount : 0;
}

double GetHighPrecisionTime() {
if (!g_timeState) return 0.0;

```
auto now = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsed = now - g_timeState->startTime;
return elapsed.count();
```

}

}
}
