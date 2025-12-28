#pragma once

#include <cstdint>

namespace Titan {
namespace Time {

void Init();
void Update(float deltaTime);

float Delta();
float DeltaUnscaled();
float Total();
float Fixed();

void SetTimeScale(float scale);
float GetTimeScale();

uint64_t GetFrameCount();

double GetHighPrecisionTime();

}
}
