#pragma once
#include "Titan_Core.hpp"
#include "Titan_SystemID.hpp"
#include "Titan_Context.hpp"

namespace Titan::ECS {

enum class SystemGroup {
    Initialization,
    Simulation,
    Presentation
};

struct ISystem {
    virtual ~ISystem() = default;
    virtual void Init(World&) {}
    virtual SystemID GetID() const = 0;
    virtual Titan::Span<const SystemID> GetDependencies() const { return {}; }
    virtual void Update(FrameContext&) = 0;
    virtual void Shutdown(World&) {}
};
}
