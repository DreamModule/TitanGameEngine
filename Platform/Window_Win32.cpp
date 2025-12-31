#include “Window.hpp”

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace Titan {
namespace Platform {
namespace Window {

namespace {
struct WindowState {
HWND hwnd = nullptr;
HDC hdc = nullptr;
HINSTANCE hInstance = nullptr;

```
    uint32_t width = 1280;
    uint32_t height = 720;
    bool shouldClose = false;
    bool isFullscreen = false;
    bool vsync = true;
    
    EventCallback eventCallback;
    
    DWORD savedStyle = 0;
    RECT savedRect = {};
};

WindowState* g_window = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!g_window) return DefWindowProcW(hwnd, msg, wParam, lParam);
    
    Event event;
    
    switch (msg) {
        case WM_CLOSE: {
            event.type = EventType::WindowClose;
            g_window->shouldClose = true;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_SIZE: {
            g_window->width = LOWORD(lParam);
            g_window->height = HIWORD(lParam);
            event.type = EventType::WindowResize;
            event.resize.width = g_window->width;
            event.resize.height = g_window->height;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_SETFOCUS: {
            event.type = EventType::WindowFocus;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_KILLFOCUS: {
            event.type = EventType::WindowLostFocus;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            event.type = EventType::KeyPress;
            event.key.keycode = static_cast<uint32_t>(wParam);
            event.key.repeat = (lParam & 0x40000000) != 0;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            event.type = EventType::KeyRelease;
            event.key.keycode = static_cast<uint32_t>(wParam);
            event.key.repeat = false;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            event.type = EventType::MouseButtonPress;
            event.mouseButton.button = (msg == WM_LBUTTONDOWN) ? 0 : 
                                      (msg == WM_RBUTTONDOWN) ? 1 : 2;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            event.type = EventType::MouseButtonRelease;
            event.mouseButton.button = (msg == WM_LBUTTONUP) ? 0 : 
                                      (msg == WM_RBUTTONUP) ? 1 : 2;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            event.type = EventType::MouseMove;
            event.mouseMove.x = GET_X_LPARAM(lParam);
            event.mouseMove.y = GET_Y_LPARAM(lParam);
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_MOUSEWHEEL: {
            event.type = EventType::MouseScroll;
            event.mouseScroll.xOffset = 0.0f;
            event.mouseScroll.yOffset = GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
            if (g_window->eventCallback) g_window->eventCallback(event);
            return 0;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
```

}

bool Create(const CreateInfo& info) {
if (g_window) return false;

```
g_window = new WindowState();
g_window->width = info.width;
g_window->height = info.height;
g_window->vsync = info.vsync;
g_window->hInstance = GetModuleHandle(nullptr);

WNDCLASSEXW wc = {};
wc.cbSize = sizeof(WNDCLASSEXW);
wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
wc.lpfnWndProc = WndProc;
wc.hInstance = g_window->hInstance;
wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
wc.lpszClassName = L"TitanEngineWindowClass";

if (!RegisterClassExW(&wc)) {
    delete g_window;
    g_window = nullptr;
    return false;
}

DWORD style = WS_OVERLAPPEDWINDOW;
if (!info.resizable) {
    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
}

RECT rect = { 0, 0, (LONG)info.width, (LONG)info.height };
AdjustWindowRect(&rect, style, FALSE);

int width = rect.right - rect.left;
int height = rect.bottom - rect.top;

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int posX = (screenWidth - width) / 2;
int posY = (screenHeight - height) / 2;

int titleLen = MultiByteToWideChar(CP_UTF8, 0, info.title.c_str(), -1, nullptr, 0);
wchar_t* titleW = new wchar_t[titleLen];
MultiByteToWideChar(CP_UTF8, 0, info.title.c_str(), -1, titleW, titleLen);

g_window->hwnd = CreateWindowExW(
    0,
    L"TitanEngineWindowClass",
    titleW,
    style,
    posX, posY,
    width, height,
    nullptr,
    nullptr,
    g_window->hInstance,
    nullptr
);

delete[] titleW;

if (!g_window->hwnd) {
    delete g_window;
    g_window = nullptr;
    return false;
}

g_window->hdc = GetDC(g_window->hwnd);

ShowWindow(g_window->hwnd, SW_SHOW);
UpdateWindow(g_window->hwnd);

if (info.fullscreen) {
    SetFullscreen(true);
}

return true;
```

}

void Destroy() {
if (!g_window) return;

```
if (g_window->hwnd) {
    if (g_window->hdc) {
        ReleaseDC(g_window->hwnd, g_window->hdc);
    }
    DestroyWindow(g_window->hwnd);
}

delete g_window;
g_window = nullptr;
```

}

void PollEvents() {
if (!g_window) return;

```
MSG msg;
while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
        g_window->shouldClose = true;
    }
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
}
```

}

bool ShouldClose() {
return g_window ? g_window->shouldClose : true;
}

void SetEventCallback(EventCallback callback) {
if (g_window) {
g_window->eventCallback = callback;
}
}

void SetTitle(const std::string& title) {
if (!g_window || !g_window->hwnd) return;

```
int len = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
wchar_t* titleW = new wchar_t[len];
MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, titleW, len);
SetWindowTextW(g_window->hwnd, titleW);
delete[] titleW;
```

}

void SetSize(uint32_t width, uint32_t height) {
if (!g_window || !g_window->hwnd) return;

```
RECT rect = { 0, 0, (LONG)width, (LONG)height };
DWORD style = GetWindowLong(g_window->hwnd, GWL_STYLE);
AdjustWindowRect(&rect, style, FALSE);

SetWindowPos(g_window->hwnd, nullptr, 0, 0, 
             rect.right - rect.left, rect.bottom - rect.top,
             SWP_NOMOVE | SWP_NOZORDER);
```

}

void SetFullscreen(bool fullscreen) {
if (!g_window || !g_window->hwnd) return;
if (g_window->isFullscreen == fullscreen) return;

```
if (fullscreen) {
    g_window->savedStyle = GetWindowLong(g_window->hwnd, GWL_STYLE);
    GetWindowRect(g_window->hwnd, &g_window->savedRect);
    
    SetWindowLong(g_window->hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    SetWindowPos(g_window->hwnd, HWND_TOP, 0, 0, 
                 screenWidth, screenHeight,
                 SWP_FRAMECHANGED);
} else {
    SetWindowLong(g_window->hwnd, GWL_STYLE, g_window->savedStyle);
    
    SetWindowPos(g_window->hwnd, nullptr,
                 g_window->savedRect.left,
                 g_window->savedRect.top,
                 g_window->savedRect.right - g_window->savedRect.left,
                 g_window->savedRect.bottom - g_window->savedRect.top,
                 SWP_FRAMECHANGED);
}

g_window->isFullscreen = fullscreen;
```

}

void SetVSync(bool enabled) {
if (g_window) {
g_window->vsync = enabled;
}
}

uint32_t GetWidth() {
return g_window ? g_window->width : 0;
}

uint32_t GetHeight() {
return g_window ? g_window->height : 0;
}

float GetAspectRatio() {
if (!g_window || g_window->height == 0) return 16.0f / 9.0f;
return static_cast<float>(g_window->width) / static_cast<float>(g_window->height);
}

bool IsFullscreen() {
return g_window ? g_window->isFullscreen : false;
}

void* GetNativeHandle() {
return g_window ? g_window->hdc : nullptr;
}

void* GetNativeWindowHandle() {
return g_window ? g_window->hwnd : nullptr;
}

void SwapBuffers() {
if (g_window && g_window->hdc) {
::SwapBuffers(g_window->hdc);
}
}

bool IsWindows() { return true; }
bool IsAndroid() { return false; }
bool IsLinux() { return false; }

}
}
}

#endif
