#pragma once
#include "ISystem.hpp"
#include <memory>
#include <vector>

namespace Titan::Core {

struct Scheduler {
    void AddSystem(std::unique_ptr<ISystem> sys);
    void UpdateAll(FrameContext& ctx);
    void Clear();
private:
    std::vector<std::unique_ptr<ISystem>> systems;
};

}
