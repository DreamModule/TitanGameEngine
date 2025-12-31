#include “Input.hpp”
#include <array>
#include <unordered_map>
#include <algorithm>

namespace Titan {
namespace Platform {
namespace Input {

namespace {
constexpr size_t MAX_KEYS = 512;
constexpr size_t MAX_MOUSE_BUTTONS = 8;
constexpr size_t MAX_TOUCHES = 10;

```
struct InputState {
    std::array<bool, MAX_KEYS> keysDown{};
    std::array<bool, MAX_KEYS> keysPressed{};
    std::array<bool, MAX_KEYS> keysReleased{};
    
    std::array<bool, MAX_MOUSE_BUTTONS> mouseButtonsDown{};
    std::array<bool, MAX_MOUSE_BUTTONS> mouseButtonsPressed{};
    std::array<bool, MAX_MOUSE_BUTTONS> mouseButtonsReleased{};
    
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    int32_t lastMouseX = 0;
    int32_t lastMouseY = 0;
    int32_t mouseDeltaX = 0;
    int32_t mouseDeltaY = 0;
    
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    
    bool cursorVisible = true;
    bool cursorLocked = false;
    
    struct Touch {
        bool active = false;
        float x = 0.0f;
        float y = 0.0f;
    };
    std::array<Touch, MAX_TOUCHES> touches{};
    uint32_t activeTouchCount = 0;
};

InputState* g_input = nullptr;

uint32_t MapKeyToIndex(Key key) {
    return static_cast<uint32_t>(key);
}

uint32_t MapMouseButtonToIndex(MouseButton button) {
    return static_cast<uint32_t>(button);
}
```

}

void Init() {
if (!g_input) {
g_input = new InputState();
}
}

void Update() {
if (!g_input) return;

```
g_input->keysPressed.fill(false);
g_input->keysReleased.fill(false);

g_input->mouseButtonsPressed.fill(false);
g_input->mouseButtonsReleased.fill(false);

g_input->mouseDeltaX = g_input->mouseX - g_input->lastMouseX;
g_input->mouseDeltaY = g_input->mouseY - g_input->lastMouseY;
g_input->lastMouseX = g_input->mouseX;
g_input->lastMouseY = g_input->mouseY;

g_input->scrollX = 0.0f;
g_input->scrollY = 0.0f;
```

}

void Shutdown() {
if (g_input) {
delete g_input;
g_input = nullptr;
}
}

bool IsKeyDown(Key key) {
if (!g_input) return false;
uint32_t index = MapKeyToIndex(key);
if (index >= MAX_KEYS) return false;
return g_input->keysDown[index];
}

bool IsKeyPressed(Key key) {
if (!g_input) return false;
uint32_t index = MapKeyToIndex(key);
if (index >= MAX_KEYS) return false;
return g_input->keysPressed[index];
}

bool IsKeyReleased(Key key) {
if (!g_input) return false;
uint32_t index = MapKeyToIndex(key);
if (index >= MAX_KEYS) return false;
return g_input->keysReleased[index];
}

bool IsMouseButtonDown(MouseButton button) {
if (!g_input) return false;
uint32_t index = MapMouseButtonToIndex(button);
if (index >= MAX_MOUSE_BUTTONS) return false;
return g_input->mouseButtonsDown[index];
}

bool IsMouseButtonPressed(MouseButton button) {
if (!g_input) return false;
uint32_t index = MapMouseButtonToIndex(button);
if (index >= MAX_MOUSE_BUTTONS) return false;
return g_input->mouseButtonsPressed[index];
}

bool IsMouseButtonReleased(MouseButton button) {
if (!g_input) return false;
uint32_t index = MapMouseButtonToIndex(button);
if (index >= MAX_MOUSE_BUTTONS) return false;
return g_input->mouseButtonsReleased[index];
}

void GetMousePosition(int32_t& x, int32_t& y) {
if (g_input) {
x = g_input->mouseX;
y = g_input->mouseY;
} else {
x = 0;
y = 0;
}
}

int32_t GetMouseX() {
return g_input ? g_input->mouseX : 0;
}

int32_t GetMouseY() {
return g_input ? g_input->mouseY : 0;
}

void GetMouseDelta(int32_t& dx, int32_t& dy) {
if (g_input) {
dx = g_input->mouseDeltaX;
dy = g_input->mouseDeltaY;
} else {
dx = 0;
dy = 0;
}
}

int32_t GetMouseDeltaX() {
return g_input ? g_input->mouseDeltaX : 0;
}

int32_t GetMouseDeltaY() {
return g_input ? g_input->mouseDeltaY : 0;
}

void GetMouseScroll(float& x, float& y) {
if (g_input) {
x = g_input->scrollX;
y = g_input->scrollY;
} else {
x = 0.0f;
y = 0.0f;
}
}

float GetMouseScrollX() {
return g_input ? g_input->scrollX : 0.0f;
}

float GetMouseScrollY() {
return g_input ? g_input->scrollY : 0.0f;
}

void SetCursorVisible(bool visible) {
if (g_input) {
g_input->cursorVisible = visible;
#ifdef _WIN32
ShowCursor(visible ? TRUE : FALSE);
#endif
}
}

void SetCursorLocked(bool locked) {
if (g_input) {
g_input->cursorLocked = locked;
#ifdef _WIN32
if (locked) {
RECT rect;
GetClientRect(GetForegroundWindow(), &rect);
ClientToScreen(GetForegroundWindow(), (POINT*)&rect.left);
ClientToScreen(GetForegroundWindow(), (POINT*)&rect.right);
ClipCursor(&rect);
} else {
ClipCursor(nullptr);
}
#endif
}
}

bool IsCursorVisible() {
return g_input ? g_input->cursorVisible : true;
}

bool IsCursorLocked() {
return g_input ? g_input->cursorLocked : false;
}

uint32_t GetTouchCount() {
return g_input ? g_input->activeTouchCount : 0;
}

bool IsTouchDown(uint32_t touchId) {
if (!g_input || touchId >= MAX_TOUCHES) return false;
return g_input->touches[touchId].active;
}

void GetTouchPosition(uint32_t touchId, float& x, float& y) {
if (g_input && touchId < MAX_TOUCHES) {
x = g_input->touches[touchId].x;
y = g_input->touches[touchId].y;
} else {
x = 0.0f;
y = 0.0f;
}
}

void OnKeyEvent(uint32_t keycode, bool pressed, bool repeat) {
if (!g_input || keycode >= MAX_KEYS) return;

```
if (pressed) {
    if (!g_input->keysDown[keycode] && !repeat) {
        g_input->keysPressed[keycode] = true;
    }
    g_input->keysDown[keycode] = true;
} else {
    if (g_input->keysDown[keycode]) {
        g_input->keysReleased[keycode] = true;
    }
    g_input->keysDown[keycode] = false;
}
```

}

void OnMouseButtonEvent(uint32_t button, bool pressed, int32_t x, int32_t y) {
if (!g_input || button >= MAX_MOUSE_BUTTONS) return;

```
g_input->mouseX = x;
g_input->mouseY = y;

if (pressed) {
    if (!g_input->mouseButtonsDown[button]) {
        g_input->mouseButtonsPressed[button] = true;
    }
    g_input->mouseButtonsDown[button] = true;
} else {
    if (g_input->mouseButtonsDown[button]) {
        g_input->mouseButtonsReleased[button] = true;
    }
    g_input->mouseButtonsDown[button] = false;
}
```

}

void OnMouseMoveEvent(int32_t x, int32_t y) {
if (!g_input) return;
g_input->mouseX = x;
g_input->mouseY = y;
}

void OnMouseScrollEvent(float xOffset, float yOffset) {
if (!g_input) return;
g_input->scrollX = xOffset;
g_input->scrollY = yOffset;
}

void OnTouchEvent(uint32_t id, float x, float y, bool pressed) {
if (!g_input || id >= MAX_TOUCHES) return;

```
if (pressed) {
    if (!g_input->touches[id].active) {
        g_input->activeTouchCount++;
    }
    g_input->touches[id].active = true;
    g_input->touches[id].x = x;
    g_input->touches[id].y = y;
} else {
    if (g_input->touches[id].active) {
        g_input->activeTouchCount--;
    }
    g_input->touches[id].active = false;
}
```

}

}
}
}
