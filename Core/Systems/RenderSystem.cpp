#include "RenderSystem.hpp"
#include "../../Graphics/GraphicsDevice.hpp"
#include "../../Platform/Window.hpp"
#include "../../Input/InputManager.hpp"
#include <gl/GL.h>

namespace Titan::Core::Systems {

void RenderSystem::Update(Titan::Core::FrameContext& ctx) {
    static float tx = 0.0f;
    static float ty = 0.0f;
    float speed = 300.0f;
    if (ctx.dt > 0.0f) {
        if (Titan::Input::Manager::IsKeyDown(0x25)) tx -= speed * ctx.dt;
        if (Titan::Input::Manager::IsKeyDown(0x27)) tx += speed * ctx.dt;
        if (Titan::Input::Manager::IsKeyDown(0x26)) ty += speed * ctx.dt;
        if (Titan::Input::Manager::IsKeyDown(0x28)) ty -= speed * ctx.dt;
    }

    Titan::Graphics::Device::BeginFrame();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(tx / 640.0f, ty / 360.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    glColor3f(0.9f, 0.2f, 0.2f);
    glVertex2f(-0.1f, -0.1f);
    glColor3f(0.2f, 0.9f, 0.2f);
    glVertex2f(0.1f, -0.1f);
    glColor3f(0.2f, 0.2f, 0.9f);
    glVertex2f(0.0f, 0.1f);
    glEnd();

    Titan::Graphics::Device::EndFrame();
    Titan::Platform::Window::SwapBuffers();
}

}
