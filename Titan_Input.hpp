/**
 * Titan Input Header
 * 
 * Input management and key mapping
 */

#ifndef TITAN_INPUT_HPP
#define TITAN_INPUT_HPP

#include "Titan_Core.hpp"
#include <unordered_map>
#include <string>

namespace Titan {

// ============================================================================
// Key Codes
// ============================================================================

enum class KeyCode : uint32
{
    Unknown = 0,
    
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11, F12,
    
    // Control keys
    Space,
    Escape,
    Enter,
    Tab,
    Backspace,
    Delete,
    Insert,
    Home,
    End,
    PageUp,
    PageDown,
    
    // Arrow keys
    Left,
    Right,
    Up,
    Down,
    
    // Modifiers
    LeftShift,
    RightShift,
    LeftCtrl,
    RightCtrl,
    LeftAlt,
    RightAlt,
    
    // Mouse
    MouseLeft,
    MouseRight,
    MouseMiddle,
    
    COUNT
};

// ============================================================================
// Input Manager
// ============================================================================

struct InputManager
{
    void Init();
    void Update();
    void Shutdown();

    // Action mapping
    void MapAction(const char* Name, KeyCode Key);
    void MapAxis(const char* Name, KeyCode Positive, KeyCode Negative);
    void UnmapAction(const char* Name);

    // State queries
    bool GetAction(const char* Name) const;
    bool GetActionDown(const char* Name) const;
    bool GetActionUp(const char* Name) const;
    float GetAxis(const char* Name) const;

    // Axis injection (for joysticks)
    void InjectAxis(const char* Name, float Value);

    // Pointer/Touch support
    static constexpr uint32 MAX_POINTERS = 10;
    Math::Vec2 GetPointerPosition(uint32 ID) const;
    bool GetPointerDown(uint32 ID) const;
    bool GetPointerUp(uint32 ID) const;

private:
    struct ActionBinding
    {
        KeyCode Key;
        bool WasDown;
        bool IsDown;
    };

    struct AxisBinding
    {
        KeyCode Positive;
        KeyCode Negative;
        float InjectedValue;
    };

    std::unordered_map<std::string, ActionBinding> Actions;
    std::unordered_map<std::string, AxisBinding> Axes;
    Math::Vec2 PointerPositions[MAX_POINTERS];
    bool PointerStates[MAX_POINTERS];
    bool PointerPrevStates[MAX_POINTERS];
};

// ============================================================================
// Static Input Manager API
// ============================================================================

namespace Input {

struct Manager
{
    static void Init();
    static void Update();
    static void Shutdown();

    // Action API
    static void MapAction(const char* Name, KeyCode Key);
    static void MapAxis(const char* Name, KeyCode Positive, KeyCode Negative);
    static bool GetAction(const char* Name);
    static bool GetActionDown(const char* Name);
    static bool GetActionUp(const char* Name);
    static float GetAxis(const char* Name);
    static void InjectAxis(const char* Name, float Value);

    // Pointer API
    static Math::Vec2 GetPointerPosition(uint32 ID);
    static bool GetPointerDown(uint32 ID);
    static bool GetPointerUp(uint32 ID);
};

} // namespace Input

} // namespace Titan

#endif // TITAN_INPUT_HPP
