#pragma once
#include <cstdint>

namespace Titan::ECS::Components {

struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float sx = 1.0f;
    float sy = 1.0f;
    float rot = 0.0f;
};

}
