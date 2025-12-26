#pragma once
#include "Entity.hpp"
#include "Components/Transform.hpp"
#include "Components/Sprite.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace Titan::ECS {

struct World {
    Entity CreateEntity();
    void DestroyEntity(Entity e);
    std::vector<Entity> GetAllEntities() const;

    std::unordered_map<Entity, Components::Transform> transforms;
    std::unordered_map<Entity, Components::Sprite> sprites;

private:
    Entity nextId = 1;
};

}
