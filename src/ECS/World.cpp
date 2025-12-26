#include "World.hpp"

namespace Titan::ECS {

Entity World::CreateEntity() {
    Entity e = nextId++;
    return e;
}

void World::DestroyEntity(Entity e) {
    transforms.erase(e);
    sprites.erase(e);
}

std::vector<Entity> World::GetAllEntities() const {
    std::vector<Entity> out;
    out.reserve(nextId);
    for (auto const & p : transforms) out.push_back(p.first);
    for (auto const & p : sprites) {
        if (std::find(out.begin(), out.end(), p.first) == out.end())
            out.push_back(p.first);
    }
    return out;
}

}
