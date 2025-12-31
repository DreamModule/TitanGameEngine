#include “TickManager.hpp”
#include <algorithm>

namespace Titan::Core {

TickManager::TickManager()
: tickRate(64.0f)
, tickDeltaTime(1.0f / 64.0f)
, accumulator(0.0f)
, timeScale(1.0f)
, currentTick(0)
, pendingTicks(0)
, maxTicksPerFrame(10)
, initialized(false)
, tickCallback(nullptr) {
}

void TickManager::Init(float rate) {
if (rate <= 0.0f) rate = 64.0f;

```
tickRate = rate;
tickDeltaTime = 1.0f / rate;
accumulator = 0.0f;
currentTick = 0;
pendingTicks = 0;
timeScale = 1.0f;
initialized = true;
```

}

void TickManager::Reset() {
accumulator = 0.0f;
currentTick = 0;
pendingTicks = 0;
}

void TickManager::Update(float deltaTime) {
if (!initialized) return;

```
float scaledDelta = deltaTime * timeScale;
accumulator += scaledDelta;

pendingTicks = 0;
while (accumulator >= tickDeltaTime && pendingTicks < maxTicksPerFrame) {
    if (tickCallback) {
        tickCallback(currentTick, tickDeltaTime);
    }
    
    accumulator -= tickDeltaTime;
    currentTick++;
    pendingTicks++;
}

if (accumulator >= tickDeltaTime * maxTicksPerFrame) {
    accumulator = tickDeltaTime * (maxTicksPerFrame - 1);
}
```

}

void TickManager::SetTickCallback(TickCallback callback) {
tickCallback = callback;
}

bool TickManager::ShouldTick() const {
return initialized && accumulator >= tickDeltaTime;
}

void TickManager::ConsumeTick() {
if (ShouldTick()) {
accumulator -= tickDeltaTime;
const_cast<TickManager*>(this)->currentTick++;
}
}

float TickManager::GetInterpolationAlpha() const {
if (tickDeltaTime <= 0.0f) return 0.0f;
return std::min(1.0f, accumulator / tickDeltaTime);
}

void TickManager::SetTimeScale(float scale) {
timeScale = std::max(0.0f, scale);
}

}
