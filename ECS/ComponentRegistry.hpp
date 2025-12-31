/**
 * ECS Component Registry Header
 * 
 * This file provides backward compatibility with the new unified ECS in Titan_ECS.hpp
 */

#ifndef SRC_ECS_COMPONENT_REGISTRY_HPP
#define SRC_ECS_COMPONENT_REGISTRY_HPP

#include "../../Titan_ECS.hpp"

namespace Titan::ECS {

// Component registration is now automatic via std::type_index in FWorld
// Components are registered on first use with AddComponent<T>() or RegisterComponent<T>()

} // namespace Titan::ECS

#endif // SRC_ECS_COMPONENT_REGISTRY_HPP
