#include "JoystickSystem.hpp"
#include "../../Platform/Window.hpp"
#include "../../Input/Joystick.hpp"
#include <gl/GL.h>

namespace Titan::Core::Systems {

void DrawCircle(float cx, float cy, float r, int segments = 32) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / (float)segments * 3.1415926f * 2.0f;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}

void JoystickUISystem::Update(Titan::Core::FrameContext& ctx) {
    (void)ctx;
    if (!Titan::Platform::Window::IsAndroid()) {
        Titan::Input::Joystick::Update();
        return;
    }

    Titan::Input::Joystick::Update();

    auto &vj = Titan::Input::Joystick::GetVirtual();
    if (!vj.enabled) return;

    HWND hwnd = Titan::Platform::Window::GetHWND();
    RECT rc;
    int w = 1280, h = 720;
    if (hwnd && GetClientRect(hwnd, &rc)) {
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor4f(0.1f, 0.1f, 0.1f, 0.6f);
    DrawCircle(vj.centerX, vj.centerY, vj.radius);
    glColor4f(0.8f, 0.8f, 0.8f, 0.9f);
    DrawCircle(vj.centerX + vj.stickX * vj.radius, vj.centerY + vj.stickY * vj.radius, vj.radius * 0.4f);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

}
