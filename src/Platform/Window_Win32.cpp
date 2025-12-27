#include "Window.hpp"

namespace {
    HWND g_hwnd = nullptr;
    HDC  g_hdc  = nullptr;
    bool g_shouldClose = false;

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CLOSE:
            g_shouldClose = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
}

namespace Titan::Platform::Window {

bool Create(int width, int height, const char* title) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TitanWindowClass";

    if (!RegisterClass(&wc))
        return false;

    g_hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        nullptr, nullptr,
        hInstance,
        nullptr
    );

    if (!g_hwnd)
        return false;

    g_hdc = GetDC(g_hwnd);
    return g_hdc != nullptr;
}

void PollEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            g_shouldClose = true;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool ShouldClose() {
    return g_shouldClose;
}

void SwapBuffers() {
    ::SwapBuffers(g_hdc);
}

void Destroy() {
    if (g_hwnd) {
        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
        g_hdc = nullptr;
    }
}

HWND GetHWND() {
    return g_hwnd;
}

HDC GetHDC() {
    return g_hdc;
}

bool IsAndroid() {
    return false;
}

}
