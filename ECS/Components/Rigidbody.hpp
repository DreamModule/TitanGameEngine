/**
 * Rigidbody Component
 * 
 * Backward compatibility - main definition is FRigidbodyComponent in Titan_ECS.hpp
 */

#ifndef SRC_ECS_COMPONENTS_RIGIDBODY_HPP
#define SRC_ECS_COMPONENTS_RIGIDBODY_HPP

#include "../../../Titan_ECS.hpp"

namespace Titan::ECS {

// Use FRigidbodyComponent from Titan_ECS.hpp
using RigidbodyComponent = FRigidbodyComponent;

} // namespace Titan::ECS

#endif // SRC_ECS_COMPONENTS_RIGIDBODY_HPP
