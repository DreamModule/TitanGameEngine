#include "InputManager.hpp"
#include <array>

namespace Titan::Input {

static std::array<bool, 256> g_down{};
static std::array<bool, 256> g_pressed{};
static int g_mouseX = 0;
static int g_mouseY = 0;

void Manager::OnKeyDown(uint32_t vk) {
    if (vk < 256) {
        if (!g_down[vk]) g_pressed[vk] = true;
        g_down[vk] = true;
    }
}

void Manager::OnKeyUp(uint32_t vk) {
    if (vk < 256) {
        g_down[vk] = false;
    }
}

bool Manager::IsKeyDown(uint32_t vk) {
    if (vk < 256) return g_down[vk];
    return false;
}

bool Manager::IsKeyPressed(uint32_t vk) {
    if (vk < 256) return g_pressed[vk];
    return false;
}

void Manager::OnMouseMove(int x, int y) {
    g_mouseX = x;
    g_mouseY = y;
}

int Manager::GetMouseX() {
    return g_mouseX;
}

int Manager::GetMouseY() {
    return g_mouseY;
}

void Manager::EndFrame() {
    g_pressed.fill(false);
}

}
