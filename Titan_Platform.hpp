#ifndef TITAN_PLATFORM_HPP
#define TITAN_PLATFORM_HPP
#include "Titan_Core.hpp"
#include "Titan_Input.hpp"
namespace Titan::Platform {
    void Init(u32 w, u32 h, const char* t);
    bool PumpMessages();
    void SwapBuffers();
    void Shutdown();
    bool GetKeyDown(KeyCode k);
    Math::Vec2 GetMousePos();
    f32 GetTime(); // QPC High Res
}
#endif
