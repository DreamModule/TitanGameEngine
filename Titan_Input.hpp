#pragma once
#include "Titan_Core.hpp"

namespace Titan {

enum class KeyCode : u32 {
    Unknown = 0
};

struct InputManager {
    void Init();
    void Update();
    bool GetAction(const char*) const;
    bool GetActionDown(const char*) const;
    bool GetActionUp(const char*) const;
    f32 GetAxis(const char*) const;
};
}
