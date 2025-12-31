/**
 * ECS System Header
 * 
 * This file provides backward compatibility with the new unified ECS in Titan_ECS.hpp
 */

#ifndef SRC_ECS_SYSTEM_HPP
#define SRC_ECS_SYSTEM_HPP

#include "../../Titan_ECS.hpp"

namespace Titan::ECS {

// ISystem is now defined in Titan_ECS.hpp
// Inherit from ISystem and implement:
//   void Init(FWorld& World) override;
//   void Update(FWorld& World, float DeltaTime) override;
//   void Shutdown(FWorld& World) override;
//   int GetPriority() const override;

} // namespace Titan::ECS

#endif // SRC_ECS_SYSTEM_HPP
