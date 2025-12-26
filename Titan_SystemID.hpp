#pragma once
#include <cstdint>

namespace Titan {
using SystemID = uint32_t;

namespace Sys {
static constexpr SystemID None          = 0x0000;
static constexpr SystemID InputWrapper  = 0x0001;
static constexpr SystemID PlayerControl = 0x0002;
static constexpr SystemID Physics       = 0x0003;
static constexpr SystemID Lifetime      = 0x0004;
static constexpr SystemID Snapshot      = 0x0005;
static constexpr SystemID Render        = 0x0006;
static constexpr SystemID UI_Render     = 0x0007;
static constexpr SystemID UI_Input      = 0x0008;
static constexpr SystemID _GameBegin    = 0x1000;
}
}
