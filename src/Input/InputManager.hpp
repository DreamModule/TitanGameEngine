#pragma once
#include <cstdint>

namespace Titan::Input {

struct Manager {
    static void OnKeyDown(uint32_t vk);
    static void OnKeyUp(uint32_t vk);
    static bool IsKeyDown(uint32_t vk);
    static bool IsKeyPressed(uint32_t vk);
    static void OnMouseMove(int x, int y);
    static int GetMouseX();
    static int GetMouseY();
    static void EndFrame();
};

}
