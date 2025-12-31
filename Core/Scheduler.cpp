#include "Scheduler.hpp"

namespace Titan::Core {

void Scheduler::AddSystem(std::unique_ptr<ISystem> sys) {
    systems.push_back(std::move(sys));
}

void Scheduler::UpdateAll(FrameContext& ctx) {
    for (auto& s : systems) {
        s->Update(ctx);
    }
}

void Scheduler::Clear() {
    systems.clear();
}

}
