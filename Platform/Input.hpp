#pragma once

#include <cstdint>

namespace Titan {
namespace Platform {
namespace Input {

enum class Key {
Unknown = 0,

```
Space = 32,
Apostrophe = 39,
Comma = 44,
Minus = 45,
Period = 46,
Slash = 47,

Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

Semicolon = 59,
Equal = 61,

A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

LeftBracket = 91,
Backslash = 92,
RightBracket = 93,
GraveAccent = 96,

Escape = 256,
Enter = 257,
Tab = 258,
Backspace = 259,
Insert = 260,
Delete = 261,

Right = 262, Left = 263, Down = 264, Up = 265,

PageUp = 266,
PageDown = 267,
Home = 268,
End = 269,

CapsLock = 280,
ScrollLock = 281,
NumLock = 282,
PrintScreen = 283,
Pause = 284,

F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

Keypad0 = 320, Keypad1, Keypad2, Keypad3, Keypad4,
Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
KeypadDecimal = 330,
KeypadDivide = 331,
KeypadMultiply = 332,
KeypadSubtract = 333,
KeypadAdd = 334,
KeypadEnter = 335,
KeypadEqual = 336,

LeftShift = 340,
LeftControl = 341,
LeftAlt = 342,
LeftSuper = 343,
RightShift = 344,
RightControl = 345,
RightAlt = 346,
RightSuper = 347,
Menu = 348
```

};

enum class MouseButton {
Left = 0,
Right = 1,
Middle = 2,
Button4 = 3,
Button5 = 4
};

void Init();
void Update();
void Shutdown();

bool IsKeyDown(Key key);
bool IsKeyPressed(Key key);
bool IsKeyReleased(Key key);

bool IsMouseButtonDown(MouseButton button);
bool IsMouseButtonPressed(MouseButton button);
bool IsMouseButtonReleased(MouseButton button);

void GetMousePosition(int32_t& x, int32_t& y);
int32_t GetMouseX();
int32_t GetMouseY();

void GetMouseDelta(int32_t& dx, int32_t& dy);
int32_t GetMouseDeltaX();
int32_t GetMouseDeltaY();

void GetMouseScroll(float& x, float& y);
float GetMouseScrollX();
float GetMouseScrollY();

void SetCursorVisible(bool visible);
void SetCursorLocked(bool locked);
bool IsCursorVisible();
bool IsCursorLocked();

uint32_t GetTouchCount();
bool IsTouchDown(uint32_t touchId);
void GetTouchPosition(uint32_t touchId, float& x, float& y);

void OnKeyEvent(uint32_t keycode, bool pressed, bool repeat);
void OnMouseButtonEvent(uint32_t button, bool pressed, int32_t x, int32_t y);
void OnMouseMoveEvent(int32_t x, int32_t y);
void OnMouseScrollEvent(float xOffset, float yOffset);
void OnTouchEvent(uint32_t id, float x, float y, bool pressed);

}
}
}
