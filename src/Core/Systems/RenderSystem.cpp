#include "RenderSystem.hpp"
#include "../../Graphics/GraphicsDevice.hpp"
#include "../../Platform/Window.hpp"

namespace Titan::Core::Systems {

void RenderSystem::Update(Titan::Core::FrameContext& ctx) {
    Titan::Graphics::Device::BeginFrame();
    Titan::Graphics::Device::EndFrame();
    Titan::Platform::Window::SwapBuffers();
}

}
