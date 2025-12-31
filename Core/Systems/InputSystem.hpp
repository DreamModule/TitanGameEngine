#pragma once
#include "../ISystem.hpp"

namespace Titan::Core::Systems {

struct InputSystem : public Titan::Core::ISystem {
    void Update(Titan::Core::FrameContext& ctx) override;
};

}
