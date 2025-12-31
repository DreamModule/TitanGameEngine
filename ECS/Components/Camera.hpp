/**
 * Camera Component
 */

#ifndef SRC_ECS_COMPONENTS_CAMERA_HPP
#define SRC_ECS_COMPONENTS_CAMERA_HPP

#include "../../../Titan_Core.hpp"

namespace Titan::ECS {

struct FCameraComponent
{
    float FOV = 60.0f;
    float Near = 0.1f;
    float Far = 1000.0f;
    float AspectRatio = 16.0f / 9.0f;
    bool bOrthographic = false;
    float OrthoSize = 10.0f;
    bool bIsActive = true;
    Math::Vec4 ClearColor{0.1f, 0.1f, 0.15f, 1.0f};
};

using CameraComponent = FCameraComponent;

} // namespace Titan::ECS

#endif // SRC_ECS_COMPONENTS_CAMERA_HPP
