#pragma once

#include <cstdint>
#include <limits>

namespace Titan {
namespace ECS {

using Entity = uint32_t;

constexpr Entity NULL_ENTITY = 0;
constexpr Entity MAX_ENTITIES = std::numeric_limits<uint32_t>::max();

constexpr uint32_t ENTITY_INDEX_BITS = 24;
constexpr uint32_t ENTITY_GENERATION_BITS = 8;

constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr uint32_t ENTITY_GENERATION_MASK = (1u << ENTITY_GENERATION_BITS) - 1u;

inline uint32_t GetEntityIndex(Entity entity) {
return entity & ENTITY_INDEX_MASK;
}

inline uint32_t GetEntityGeneration(Entity entity) {
return (entity >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK;
}

inline Entity CreateEntity(uint32_t index, uint32_t generation) {
return (generation << ENTITY_INDEX_BITS) | index;
}

inline bool IsEntityValid(Entity entity) {
return entity != NULL_ENTITY;
}

}
}
