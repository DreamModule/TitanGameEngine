#pragma once
#include <cstdint>

namespace Titan::Core {

struct EngineContext;

struct FrameContext {
    EngineContext* engine = nullptr;
    float dt = 0.0f;
};

}
