#ifndef TITAN_PHYSICS_HPP
#define TITAN_PHYSICS_HPP
#include "Titan_ECS.hpp"
#include "Titan_Math.hpp"
namespace Titan {
    struct RigidBodyComponent { Math::Vec2 velocity; Math::Vec2 acceleration; f32 mass; f32 drag; bool isStatic; bool useGravity; };
    struct ColliderComponent { int type; Math::Vec2 offset; Math::Vec2 size; bool isTrigger; std::vector<ECS::EntityID> collisions; };
    
    struct PhysicsSystem : ECS::ISystem {
        void Update(ECS::World& w, f32 dt) override {
            // а вот заглушка да
            // в 1.4 сделаю
        }
        int GetPriority() const override { return 100; }
    };
}
#endif
