#ifndef TITAN_INPUT_HPP
#define TITAN_INPUT_HPP
#include "Titan_Core.hpp"
namespace Titan {
    enum class KeyCode : u32 { 
        Unknown=0, W, A, S, D, Space, Escape, Enter, MouseLeft, MouseRight 
    };
    
    struct InputManager {
        void Init();
        void Update();
        void MapAction(const char* n, KeyCode k);
        void MapAxis(const char* n, KeyCode pos, KeyCode neg);
        
        bool GetAction(const char* n) const;
        bool GetActionDown(const char* n) const;
        f32 GetAxis(const char* n) const;
        
        // Injection (Joystick)
        void InjectAxis(const char* n, f32 val);
        
        // Multi-touch / Pointers
        static const u32 MAX_POINTERS = 10;
        Math::Vec2 GetPointerPosition(u32 id) const;
        bool GetPointerDown(u32 id) const;
        bool GetPointerUp(u32 id) const;
    };
}
#endif
