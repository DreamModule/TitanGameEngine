#pragma once
#include <cstdint>
#include <array>

namespace Titan::Input {

struct ControllerState {
    bool connected = false;
    std::array<float, 4> leftThumb = {0,0,0,0}; // x,y,rx,ry (rx,ry unused)
    std::array<float, 2> triggers = {0,0}; // left, right
    uint32_t buttons = 0;
};

struct VirtualJoystick {
    bool enabled = false;
    float centerX = 100.0f;
    float centerY = 600.0f;
    float radius = 80.0f;
    float deadzone = 8.0f;
    float stickX = 0.0f;
    float stickY = 0.0f;
    bool pressed = false;
    void SetPosition(float cx, float cy);
    void SetRadius(float r);
    void SetEnabled(bool e);
};

struct Joystick {
    static void Update();
    static ControllerState GetController(int idx);
    static bool IsControllerConnected(int idx);
    static float GetLeftAxisX(int idx);
    static float GetLeftAxisY(int idx);
    static bool IsControllerButtonDown(int idx, uint32_t mask);

    static VirtualJoystick& GetVirtual();
    static void SimulateVirtualTouchDown(float x, float y);
    static void SimulateVirtualTouchMove(float x, float y);
    static void SimulateVirtualTouchUp();
};

}
