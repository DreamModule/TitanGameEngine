#include "GraphicsDevice.hpp"
#include <gl/GL.h>

namespace {
    HGLRC g_glrc = nullptr;
}

namespace Titan::Graphics::Device {

bool Init(HWND, HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (!pf)
        return false;

    if (!SetPixelFormat(hdc, pf, &pfd))
        return false;

    g_glrc = wglCreateContext(hdc);
    if (!g_glrc)
        return false;

    if (!wglMakeCurrent(hdc, g_glrc))
        return false;

    return true;
}

void BeginFrame() {
    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EndFrame() {}

void Shutdown() {
    if (g_glrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_glrc);
        g_glrc = nullptr;
    }
}

}
