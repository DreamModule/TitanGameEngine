/**
 * Input Manager Header
 * Titan Engine
 */

#pragma once

#include <cstdint>

namespace Titan::Input {

/**
 * Static Input Manager class
 * Handles keyboard and mouse input
 */
struct Manager
{
    // Keyboard
    static void OnKeyDown(uint32_t VirtualKey);
    static void OnKeyUp(uint32_t VirtualKey);
    static bool IsKeyDown(uint32_t VirtualKey);
    static bool IsKeyPressed(uint32_t VirtualKey);
    static bool IsKeyReleased(uint32_t VirtualKey);
    
    // Mouse
    static void OnMouseMove(int X, int Y);
    static void OnMouseButton(int Button, bool Down);
    static int GetMouseX();
    static int GetMouseY();
    static int GetMouseDeltaX();
    static int GetMouseDeltaY();
    static bool IsMouseButtonDown(int Button);
    static bool IsMouseButtonPressed(int Button);
    
    // Frame management
    static void EndFrame();
    static void Reset();
};

} // namespace Titan::Input
