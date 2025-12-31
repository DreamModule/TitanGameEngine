/**
 * ECS Component Manager Header
 * 
 * This file provides backward compatibility with the new unified ECS in Titan_ECS.hpp
 */

#ifndef SRC_ECS_COMPONENT_MANAGER_HPP
#define SRC_ECS_COMPONENT_MANAGER_HPP

#include "../../Titan_ECS.hpp"

namespace Titan::ECS {

// Component management is now handled by FWorld and TComponentPool in Titan_ECS.hpp
// Use:
//   World.RegisterComponent<T>();
//   World.AddComponent<T>(Entity, ...);
//   World.GetComponent<T>(Entity);
//   World.HasComponent<T>(Entity);
//   World.RemoveComponent<T>(Entity);

} // namespace Titan::ECS

#endif // SRC_ECS_COMPONENT_MANAGER_HPP
