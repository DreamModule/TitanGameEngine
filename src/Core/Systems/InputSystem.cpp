#include "InputSystem.hpp"
#include "../../Input/Joystick.hpp"

namespace Titan::Core::Systems {

void InputSystem::Update(Titan::Core::FrameContext& ctx) {
    (void)ctx;
    Titan::Input::Joystick::Update();
}

}
