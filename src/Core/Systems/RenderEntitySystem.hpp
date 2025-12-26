#pragma once
#include "../ISystem.hpp"

namespace Titan::Core::Systems {

struct RenderEntitySystem : public Titan::Core::ISystem {
    void Update(Titan::Core::FrameContext& ctx) override;
};

}
