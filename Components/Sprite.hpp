#pragma once
#include <string>
#include "../Entity.hpp"

namespace Titan::ECS::Components {

struct Sprite {
    std::string path;
    unsigned int texId = 0;
    int w = 0;
    int h = 0;
};

}
