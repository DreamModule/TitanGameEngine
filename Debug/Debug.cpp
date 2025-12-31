#include "Debug.hpp"
#include "../Platform/Window.hpp"
#include "../Input/InputManager.hpp"
#include <windows.h>
#include <gl/GL.h>
#include <string>
#include <cstring>

namespace {
    HGDIOBJ g_oldFont = nullptr;
    HFONT g_font = nullptr;
    GLuint g_fontBase = 0;
    int g_clientW = 1280;
    int g_clientH = 720;
}

namespace Titan::Debug {

void Init() {
    HWND hwnd = Titan::Platform::Window::GetHWND();
    HDC hdc = Titan::Platform::Window::GetHDC();
    RECT rc;
    if (hwnd) GetClientRect(hwnd, &rc);
    g_clientW = (rc.right - rc.left);
    g_clientH = (rc.bottom - rc.top);
    g_font = CreateFontA(16,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,"Consolas");
    g_oldFont = SelectObject(hdc, g_font);
    g_fontBase = glGenLists(256);
    wglUseFontBitmaps(hdc, 0, 256, g_fontBase);
}

void Shutdown() {
    HWND hwnd = Titan::Platform::Window::GetHWND();
    HDC hdc = Titan::Platform::Window::GetHDC();
    if (g_oldFont) SelectObject(hdc, g_oldFont);
    if (g_font) DeleteObject(g_font);
    if (g_fontBase) glDeleteLists(g_fontBase, 256);
    g_oldFont = nullptr;
    g_font = nullptr;
    g_fontBase = 0;
}

void Begin() {
    HWND hwnd = Titan::Platform::Window::GetHWND();
    RECT rc;
    if (hwnd) GetClientRect(hwnd, &rc);
    g_clientW = (rc.right - rc.left);
    g_clientH = (rc.bottom - rc.top);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_clientW, g_clientH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void End() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Text(int x, int y, const char* text) {
    if (!text) return;
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(x, y);
    glListBase(g_fontBase);
    size_t len = std::strlen(text);
    glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, (const GLubyte*)text);
}

void Rect(int x, int y, int w, int h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

bool Button(int x, int y, int w, int h, const char* text) {
    int mx = Titan::Input::Manager::GetMouseX();
    int my = Titan::Input::Manager::GetMouseY();
    bool inside = (mx >= x && mx <= x + w && my >= y && my <= y + h);
    if (inside) Rect(x, y, w, h, 0.3f, 0.5f, 0.9f, 1.0f);
    else Rect(x, y, w, h, 0.2f, 0.2f, 0.2f, 1.0f);
    Text(x + 6, y + h / 2 + 6, text);
    if (inside && Titan::Input::Manager::IsKeyPressed(VK_LBUTTON)) return true;
    return false;
}

}
