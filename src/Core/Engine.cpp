#include "Engine.hpp"
#include "../Platform/Window.hpp"
#include "../Graphics/GraphicsDevice.hpp"
#include "Scheduler.hpp"
#include "Systems/PlatformPollSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include <memory>
#include <chrono>

namespace Titan::Core::Engine {

static std::unique_ptr<Scheduler> g_scheduler;
static std::chrono::high_resolution_clock::time_point g_prevTime;

bool Init(EngineContext& ctx) {
    if (!Titan::Platform::Window::Create(1280, 720, "Titan Engine"))
        return false;

    if (!Titan::Graphics::Device::Init(
            Titan::Platform::Window::GetHWND(),
            Titan::Platform::Window::GetHDC()))
        return false;

    g_scheduler = std::make_unique<Scheduler>();
    g_scheduler->AddSystem(std::make_unique<Titan::Core::Systems::PlatformPollSystem>());
    g_scheduler->AddSystem(std::make_unique<Titan::Core::Systems::RenderSystem>());

    g_prevTime = std::chrono::high_resolution_clock::now();

    return true;
}

void Update(EngineContext& ctx) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = now - g_prevTime;
    g_prevTime = now;
    ctx.deltaTime = diff.count();

    FrameContext frameCtx;
    frameCtx.engine = &ctx;
    frameCtx.dt = ctx.deltaTime;

    if (g_scheduler) g_scheduler->UpdateAll(frameCtx);
}

void Shutdown(EngineContext& ctx) {
    if (g_scheduler) {
        g_scheduler->Clear();
        g_scheduler.reset();
    }
    Titan::Graphics::Device::Shutdown();
    Titan::Platform::Window::Destroy();
}

}
