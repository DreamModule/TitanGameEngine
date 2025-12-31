#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace Titan {
namespace Platform {
namespace Window {

enum class EventType {
None,
WindowClose,
WindowResize,
WindowFocus,
WindowLostFocus,
KeyPress,
KeyRelease,
MouseButtonPress,
MouseButtonRelease,
MouseMove,
MouseScroll,
TouchDown,
TouchMove,
TouchUp
};

struct Event {
EventType type = EventType::None;

```
union {
    struct { uint32_t width, height; } resize;
    struct { uint32_t keycode; bool repeat; } key;
    struct { uint32_t button; int32_t x, y; } mouseButton;
    struct { int32_t x, y; } mouseMove;
    struct { float xOffset, yOffset; } mouseScroll;
    struct { uint32_t id; float x, y; } touch;
};
```

};

using EventCallback = std::function<void(const Event&)>;

struct CreateInfo {
uint32_t width = 1280;
uint32_t height = 720;
std::string title = “Titan Engine”;
bool fullscreen = false;
bool resizable = true;
bool vsync = true;
void* nativeHandle = nullptr;
};

bool Create(const CreateInfo& info);
void Destroy();

void PollEvents();
bool ShouldClose();

void SetEventCallback(EventCallback callback);

void SetTitle(const std::string& title);
void SetSize(uint32_t width, uint32_t height);
void SetFullscreen(bool fullscreen);
void SetVSync(bool enabled);

uint32_t GetWidth();
uint32_t GetHeight();
float GetAspectRatio();
bool IsFullscreen();

void* GetNativeHandle();
void* GetNativeWindowHandle();

void SwapBuffers();

bool IsWindows();
bool IsAndroid();
bool IsLinux();

}
}
}
