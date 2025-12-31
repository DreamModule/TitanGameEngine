#include "Joystick.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput")
#endif
#include <algorithm>
#include <cmath>

namespace Titan::Input {

static std::array<ControllerState, 4> g_controllers;
static VirtualJoystick g_virtual;

void VirtualJoystick::SetPosition(float cx, float cy) {
    centerX = cx;
    centerY = cy;
}
void VirtualJoystick::SetRadius(float r) {
    radius = r;
}
void VirtualJoystick::SetEnabled(bool e) {
    enabled = e;
}

void Joystick::Update() {
#ifdef _WIN32
    for (DWORD i = 0; i < 4; ++i) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(state));
        DWORD res = XInputGetState(i, &state);
        if (res == ERROR_SUCCESS) {
            g_controllers[i].connected = true;
            SHORT lx = state.Gamepad.sThumbLX;
            SHORT ly = state.Gamepad.sThumbLY;
            SHORT rx = state.Gamepad.sThumbRX;
            SHORT ry = state.Gamepad.sThumbRY;
            BYTE lt = state.Gamepad.bLeftTrigger;
            BYTE rt = state.Gamepad.bRightTrigger;
            const float normL = 32767.0f;
            const float normR = 32767.0f;
            float fx = (std::abs(lx) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ? 0.0f : (float)lx / normL;
            float fy = (std::abs(ly) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ? 0.0f : (float)ly / normL;
            g_controllers[i].leftThumb[0] = fx;
            g_controllers[i].leftThumb[1] = fy;
            g_controllers[i].triggers[0] = (float)lt / 255.0f;
            g_controllers[i].triggers[1] = (float)rt / 255.0f;
            g_controllers[i].buttons = state.Gamepad.wButtons;
        } else {
            g_controllers[i].connected = false;
            g_controllers[i].leftThumb = {0,0,0,0};
            g_controllers[i].triggers = {0,0};
            g_controllers[i].buttons = 0;
        }
    }
#endif
}

ControllerState Joystick::GetController(int idx) {
    if (idx < 0 || idx >= (int)g_controllers.size()) return ControllerState{};
    return g_controllers[idx];
}

bool Joystick::IsControllerConnected(int idx) {
    if (idx < 0 || idx >= (int)g_controllers.size()) return false;
    return g_controllers[idx].connected;
}

float Joystick::GetLeftAxisX(int idx) {
    if (idx < 0 || idx >= (int)g_controllers.size()) return 0.0f;
    return g_controllers[idx].leftThumb[0];
}

float Joystick::GetLeftAxisY(int idx) {
    if (idx < 0 || idx >= (int)g_controllers.size()) return 0.0f;
    return g_controllers[idx].leftThumb[1];
}

bool Joystick::IsControllerButtonDown(int idx, uint32_t mask) {
    if (idx < 0 || idx >= (int)g_controllers.size()) return false;
    return (g_controllers[idx].buttons & mask) != 0;
}

VirtualJoystick& Joystick::GetVirtual() {
    return g_virtual;
}

void Joystick::SimulateVirtualTouchDown(float x, float y) {
    g_virtual.pressed = true;
    float dx = x - g_virtual.centerX;
    float dy = y - g_virtual.centerY;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len <= g_virtual.deadzone) { g_virtual.stickX = 0; g_virtual.stickY = 0; return; }
    if (len > g_virtual.radius) { dx = dx / len * g_virtual.radius; dy = dy / len * g_virtual.radius; len = g_virtual.radius; }
    g_virtual.stickX = dx / g_virtual.radius;
    g_virtual.stickY = dy / g_virtual.radius;
}

void Joystick::SimulateVirtualTouchMove(float x, float y) {
    if (!g_virtual.pressed) return;
    SimulateVirtualTouchDown(x,y);
}

void Joystick::SimulateVirtualTouchUp() {
    g_virtual.pressed = false;
    g_virtual.stickX = 0.0f;
    g_virtual.stickY = 0.0f;
}

}
