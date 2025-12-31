#include "PlatformPollSystem.hpp"
#include "../../Platform/Window.hpp"

namespace Titan::Core::Systems {

void PlatformPollSystem::Update(Titan::Core::FrameContext& ctx) {
    Titan::Platform::Window::PollEvents();
    if (Titan::Platform::Window::ShouldClose()) {
        if (ctx.engine) ctx.engine->running = false;
    }
}

}
