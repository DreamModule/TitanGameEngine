#include "Engine.hpp"
#include "../Platform/Window.hpp"
#include "../Graphics/GraphicsDevice.hpp"

namespace Titan::Core::Engine {

bool Init(EngineContext&) {
    if (!Titan::Platform::Window::Create(1280, 720, "Titan Engine"))
        return false;

    if (!Titan::Graphics::Device::Init(
            Titan::Platform::Window::GetHWND(),
            Titan::Platform::Window::GetHDC()))
        return false;

    return true;
}

void Update(EngineContext& ctx) {
    Titan::Platform::Window::PollEvents();
    if (Titan::Platform::Window::ShouldClose())
        ctx.running = false;

    Titan::Graphics::Device::BeginFrame();
    Titan::Graphics::Device::EndFrame();
    Titan::Platform::Window::SwapBuffers();
}

void Shutdown(EngineContext&) {
    Titan::Graphics::Device::Shutdown();
    Titan::Platform::Window::Destroy();
}

}
