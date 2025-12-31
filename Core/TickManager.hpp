#pragma once
#include <cstdint>
#include <functional>

namespace Titan::Core {

class TickManager {
public:
using TickCallback = std::function<void(uint32_t tick, float deltaTime)>;

```
TickManager();
~TickManager() = default;

void Init(float tickRate);
void Reset();

void Update(float deltaTime);

void SetTickCallback(TickCallback callback);

bool ShouldTick() const;
void ConsumeTick();

uint32_t GetCurrentTick() const { return currentTick; }
uint32_t GetPendingTicks() const { return pendingTicks; }
float GetTickRate() const { return tickRate; }
float GetTickDeltaTime() const { return tickDeltaTime; }
float GetAccumulator() const { return accumulator; }
float GetInterpolationAlpha() const;

double GetTickTime() const { return currentTick * tickDeltaTime; }

void SetMaxTicksPerFrame(uint32_t max) { maxTicksPerFrame = max; }
uint32_t GetMaxTicksPerFrame() const { return maxTicksPerFrame; }

void SetTimeScale(float scale);
float GetTimeScale() const { return timeScale; }

bool IsInitialized() const { return initialized; }
```

private:
float tickRate;
float tickDeltaTime;
float accumulator;
float timeScale;

```
uint32_t currentTick;
uint32_t pendingTicks;
uint32_t maxTicksPerFrame;

bool initialized;

TickCallback tickCallback;
```

};

}
