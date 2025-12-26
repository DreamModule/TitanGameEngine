#ifndef TITAN_INPUT_HPP
#define TITAN_INPUT_HPP

#include "Titan_Core.hpp"

namespace Titan {

enum class KeyCode : u16 {
    None=0, W, A, S, D, Space, Escape, Shift, Ctrl, Enter,
    Up, Down, Left, Right,
    MouseLeft, MouseRight, MouseMiddle,
    MouseX, MouseY, MouseScroll,
    GamepadLeftStickX, GamepadLeftStickY, GamepadFaceButtonBottom,
    VirtualStickX, VirtualStickY, Count
};

struct Input {
    struct Manager {
        static void Init();
        static void MapAction(const char* n, KeyCode k);
        static void MapAxisKeys(const char* n, KeyCode p, KeyCode ng);
        static void MapAxisDirect(const char* n, KeyCode a, f32 s=1.0f);
        static void Update();
        static bool GetActionDown(const char* n);
        static bool GetActionUp(const char* n);
        static bool GetAction(const char* n);
        static f32  GetAxis(const char* n);
        static void InjectAxis(const char* n, f32 v);
        static Math::Vec2 GetMousePosition();
    };
};
}
#endif
