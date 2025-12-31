#pragma once
#include "FrameContext.hpp"

namespace Titan::Core {

struct ISystem {
    virtual ~ISystem() = default;
    virtual void Update(FrameContext& ctx) = 0;
};

}
