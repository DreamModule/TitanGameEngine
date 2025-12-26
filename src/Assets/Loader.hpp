#pragma once
#include "Texture.hpp"

namespace Titan::Assets {

struct Loader {
    static Texture LoadTexture(const char* path);
};

}
