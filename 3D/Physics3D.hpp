/**
 * Titan 3D Physics System
 * 
 * Full-featured physics with:
 * - Rigidbody dynamics
 * - Collision detection (Sphere, Box, Capsule)
 * - Raycasting
 * - Trigger volumes
 * - Character controller support
 */

#ifndef TITAN_PHYSICS_3D_HPP
#define TITAN_PHYSICS_3D_HPP

#include "../Titan_Core.hpp"
#include "../Titan_ECS.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

namespace Titan::Physics {

// ============================================================================
// Collision Layers
// ============================================================================

using FCollisionMask = uint32_t;

namespace CollisionLayers
{
    constexpr FCollisionMask Default     = 1 << 0;
    constexpr FCollisionMask Static      = 1 << 1;
    constexpr FCollisionMask Dynamic     = 1 << 2;
    constexpr FCollisionMask Player      = 1 << 3;
    constexpr FCollisionMask Enemy       = 1 << 4;
    constexpr FCollisionMask Projectile  = 1 << 5;
    constexpr FCollisionMask Trigger     = 1 << 6;
    constexpr FCollisionMask All         = 0xFFFFFFFF;
}

// ============================================================================
// Collider Types
// ============================================================================

enum class EColliderType
{
    Sphere,
    Box,
    Capsule,
    Mesh
};

struct FSphereCollider
{
    Math::Vec3 Center{0, 0, 0};
    float Radius = 0.5f;
};

struct FBoxCollider
{
    Math::Vec3 Center{0, 0, 0};
    Math::Vec3 HalfExtents{0.5f, 0.5f, 0.5f};
    Math::Quaternion Rotation = Math::Quaternion::Identity();
};

struct FCapsuleCollider
{
    Math::Vec3 Center{0, 0, 0};
    float Radius = 0.5f;
    float Height = 2.0f; // Total height including caps
    
    // Direction: 0 = X, 1 = Y (default), 2 = Z
    int Direction = 1;
};

// ============================================================================
// Collider Component
// ============================================================================

struct FCollider3DComponent
{
    EColliderType Type = EColliderType::Sphere;
    
    FSphereCollider Sphere;
    FBoxCollider Box;
    FCapsuleCollider Capsule;
    
    // Physics material
    float Friction = 0.5f;
    float Bounciness = 0.0f;
    
    // Layers
    FCollisionMask Layer = CollisionLayers::Default;
    FCollisionMask Mask = CollisionLayers::All;
    
    // Trigger
    bool bIsTrigger = false;
    
    // Cached world-space bounds
    Math::Vec3 WorldCenter{0, 0, 0};
    float WorldRadius = 0.5f;
};

// ============================================================================
// Rigidbody Component
// ============================================================================

struct FRigidbody3DComponent
{
    // Linear motion
    Math::Vec3 Velocity{0, 0, 0};
    Math::Vec3 Acceleration{0, 0, 0};
    Math::Vec3 Force{0, 0, 0};
    
    // Angular motion
    Math::Vec3 AngularVelocity{0, 0, 0};
    Math::Vec3 Torque{0, 0, 0};
    
    // Mass
    float Mass = 1.0f;
    float InverseMass = 1.0f;
    
    // Inertia tensor (simplified - using scalar for now)
    float Inertia = 1.0f;
    float InverseInertia = 1.0f;
    
    // Damping
    float LinearDamping = 0.01f;
    float AngularDamping = 0.05f;
    
    // State
    bool bUseGravity = true;
    bool bIsKinematic = false;
    bool bIsStatic = false;
    bool bFreezeRotation = false;
    
    // Sleep
    bool bIsSleeping = false;
    float SleepThreshold = 0.1f;
    float SleepTimer = 0.0f;
    
    void SetMass(float NewMass)
    {
        Mass = NewMass;
        InverseMass = (NewMass > 0.0001f) ? (1.0f / NewMass) : 0.0f;
        
        // Simple inertia calculation (sphere approximation)
        Inertia = 0.4f * Mass; // 2/5 * m * r^2, assuming r=1
        InverseInertia = (Inertia > 0.0001f) ? (1.0f / Inertia) : 0.0f;
    }
    
    void AddForce(const Math::Vec3& F)
    {
        Force = Force + F;
        WakeUp();
    }
    
    void AddImpulse(const Math::Vec3& Impulse)
    {
        Velocity = Velocity + Impulse * InverseMass;
        WakeUp();
    }
    
    void AddTorque(const Math::Vec3& T)
    {
        Torque = Torque + T;
        WakeUp();
    }
    
    void WakeUp()
    {
        bIsSleeping = false;
        SleepTimer = 0.0f;
    }
};

// ============================================================================
// Raycast Result
// ============================================================================

struct FRaycastHit
{
    bool bHit = false;
    float Distance = 0.0f;
    Math::Vec3 Point{0, 0, 0};
    Math::Vec3 Normal{0, 1, 0};
    ECS::FEntityID Entity = ECS::NULL_ENTITY;
    FCollider3DComponent* Collider = nullptr;
};

// ============================================================================
// Contact Point
// ============================================================================

struct FContactPoint
{
    Math::Vec3 Point;
    Math::Vec3 Normal;
    float Penetration = 0.0f;
};

struct FCollisionPair
{
    ECS::FEntityID EntityA = ECS::NULL_ENTITY;
    ECS::FEntityID EntityB = ECS::NULL_ENTITY;
    std::vector<FContactPoint> Contacts;
    float ImpulseMagnitude = 0.0f;
};

// ============================================================================
// Physics World
// ============================================================================

class FPhysicsWorld
{
public:
    // Gravity
    Math::Vec3 Gravity{0, -9.81f, 0};
    
    // Simulation settings
    float FixedTimeStep = 1.0f / 60.0f;
    int MaxSubSteps = 8;
    int VelocityIterations = 8;
    int PositionIterations = 3;
    
    // Callbacks
    std::function<void(ECS::FEntityID, ECS::FEntityID, const FContactPoint&)> OnCollisionEnter;
    std::function<void(ECS::FEntityID, ECS::FEntityID)> OnCollisionExit;
    std::function<void(ECS::FEntityID, ECS::FEntityID)> OnTriggerEnter;
    std::function<void(ECS::FEntityID, ECS::FEntityID)> OnTriggerExit;

private:
    float AccumulatedTime = 0.0f;
    std::vector<FCollisionPair> CollisionPairs;
    std::vector<FCollisionPair> PreviousCollisions;
    ECS::FWorld* World = nullptr;

public:
    void Initialize(ECS::FWorld* InWorld)
    {
        World = InWorld;
    }

    void Step(float DeltaTime)
    {
        if (!World) return;

        AccumulatedTime += DeltaTime;
        int Steps = 0;

        while (AccumulatedTime >= FixedTimeStep && Steps < MaxSubSteps)
        {
            FixedStep(FixedTimeStep);
            AccumulatedTime -= FixedTimeStep;
            Steps++;
        }

        // Interpolate for smooth rendering
        float Alpha = AccumulatedTime / FixedTimeStep;
        InterpolateTransforms(Alpha);
    }

    // ========================================================================
    // Raycasting
    // ========================================================================

    FRaycastHit Raycast(const Math::Vec3& Origin, const Math::Vec3& Direction, 
                        float MaxDistance = 1000.0f, 
                        FCollisionMask Mask = CollisionLayers::All)
    {
        FRaycastHit ClosestHit;
        ClosestHit.Distance = MaxDistance;

        if (!World) return ClosestHit;

        Math::Vec3 Dir = Direction.Normalized();

        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                if (!(Collider.Layer & Mask))
                {
                    return;
                }

                FRaycastHit Hit;
                
                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        Hit = RaycastSphere(Origin, Dir, Transform.Position + Collider.Sphere.Center, 
                                           Collider.Sphere.Radius);
                        break;
                    case EColliderType::Box:
                        Hit = RaycastBox(Origin, Dir, Transform.Position + Collider.Box.Center,
                                        Collider.Box.HalfExtents, Collider.Box.Rotation);
                        break;
                    case EColliderType::Capsule:
                        Hit = RaycastCapsule(Origin, Dir, Transform.Position + Collider.Capsule.Center,
                                            Collider.Capsule.Radius, Collider.Capsule.Height,
                                            Collider.Capsule.Direction);
                        break;
                    default:
                        return;
                }

                if (Hit.bHit && Hit.Distance < ClosestHit.Distance)
                {
                    ClosestHit = Hit;
                    ClosestHit.Entity = Entity;
                    ClosestHit.Collider = &Collider;
                }
            }
        );

        ClosestHit.bHit = (ClosestHit.Distance < MaxDistance);
        return ClosestHit;
    }

    std::vector<FRaycastHit> RaycastAll(const Math::Vec3& Origin, const Math::Vec3& Direction,
                                        float MaxDistance = 1000.0f,
                                        FCollisionMask Mask = CollisionLayers::All)
    {
        std::vector<FRaycastHit> Hits;
        
        if (!World) return Hits;

        Math::Vec3 Dir = Direction.Normalized();

        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                if (!(Collider.Layer & Mask))
                {
                    return;
                }

                FRaycastHit Hit;
                
                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        Hit = RaycastSphere(Origin, Dir, Transform.Position + Collider.Sphere.Center,
                                           Collider.Sphere.Radius);
                        break;
                    case EColliderType::Box:
                        Hit = RaycastBox(Origin, Dir, Transform.Position + Collider.Box.Center,
                                        Collider.Box.HalfExtents, Collider.Box.Rotation);
                        break;
                    case EColliderType::Capsule:
                        Hit = RaycastCapsule(Origin, Dir, Transform.Position + Collider.Capsule.Center,
                                            Collider.Capsule.Radius, Collider.Capsule.Height,
                                            Collider.Capsule.Direction);
                        break;
                    default:
                        return;
                }

                if (Hit.bHit && Hit.Distance <= MaxDistance)
                {
                    Hit.Entity = Entity;
                    Hit.Collider = &Collider;
                    Hits.push_back(Hit);
                }
            }
        );

        // Sort by distance
        std::sort(Hits.begin(), Hits.end(), 
            [](const FRaycastHit& A, const FRaycastHit& B) { return A.Distance < B.Distance; });

        return Hits;
    }

    // ========================================================================
    // Overlap Tests
    // ========================================================================

    std::vector<ECS::FEntityID> OverlapSphere(const Math::Vec3& Center, float Radius,
                                               FCollisionMask Mask = CollisionLayers::All)
    {
        std::vector<ECS::FEntityID> Results;
        
        if (!World) return Results;

        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                if (!(Collider.Layer & Mask))
                {
                    return;
                }

                Math::Vec3 ColliderCenter = Transform.Position;
                
                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        ColliderCenter = ColliderCenter + Collider.Sphere.Center;
                        if (SphereSphereTest(Center, Radius, ColliderCenter, Collider.Sphere.Radius))
                        {
                            Results.push_back(Entity);
                        }
                        break;
                    case EColliderType::Box:
                        ColliderCenter = ColliderCenter + Collider.Box.Center;
                        if (SphereBoxTest(Center, Radius, ColliderCenter, Collider.Box.HalfExtents))
                        {
                            Results.push_back(Entity);
                        }
                        break;
                    case EColliderType::Capsule:
                        ColliderCenter = ColliderCenter + Collider.Capsule.Center;
                        if (SphereCapsuleTest(Center, Radius, ColliderCenter, 
                            Collider.Capsule.Radius, Collider.Capsule.Height, Collider.Capsule.Direction))
                        {
                            Results.push_back(Entity);
                        }
                        break;
                    default:
                        break;
                }
            }
        );

        return Results;
    }

    std::vector<ECS::FEntityID> OverlapBox(const Math::Vec3& Center, const Math::Vec3& HalfExtents,
                                           FCollisionMask Mask = CollisionLayers::All)
    {
        std::vector<ECS::FEntityID> Results;
        
        if (!World) return Results;

        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                if (!(Collider.Layer & Mask))
                {
                    return;
                }

                bool Hit = false;
                Math::Vec3 ColliderCenter = Transform.Position;

                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        ColliderCenter = ColliderCenter + Collider.Sphere.Center;
                        Hit = SphereBoxTest(ColliderCenter, Collider.Sphere.Radius, Center, HalfExtents);
                        break;
                    case EColliderType::Box:
                        ColliderCenter = ColliderCenter + Collider.Box.Center;
                        Hit = BoxBoxTest(Center, HalfExtents, ColliderCenter, Collider.Box.HalfExtents);
                        break;
                    default:
                        break;
                }

                if (Hit)
                {
                    Results.push_back(Entity);
                }
            }
        );

        return Results;
    }

private:
    void FixedStep(float DT)
    {
        // Apply forces and integrate
        ApplyGravity();
        IntegrateVelocities(DT);
        
        // Collision detection
        CollisionPairs.clear();
        BroadPhase();
        NarrowPhase();
        
        // Resolve collisions
        for (int i = 0; i < VelocityIterations; i++)
        {
            ResolveVelocities();
        }
        
        for (int i = 0; i < PositionIterations; i++)
        {
            ResolvePositions();
        }
        
        // Integrate positions
        IntegratePositions(DT);
        
        // Update transforms
        UpdateTransforms();
        
        // Fire callbacks
        FireCollisionCallbacks();
        
        // Clear forces
        ClearForces();
        
        // Sleep check
        UpdateSleep(DT);
    }

    void ApplyGravity()
    {
        World->Each<FRigidbody3DComponent>(
            [this](ECS::FEntityID Entity, FRigidbody3DComponent& RB)
            {
                if (!RB.bIsStatic && !RB.bIsKinematic && RB.bUseGravity && !RB.bIsSleeping)
                {
                    RB.Force = RB.Force + Gravity * RB.Mass;
                }
            }
        );
    }

    void IntegrateVelocities(float DT)
    {
        World->Each<FRigidbody3DComponent>(
            [DT](ECS::FEntityID Entity, FRigidbody3DComponent& RB)
            {
                if (RB.bIsStatic || RB.bIsKinematic || RB.bIsSleeping)
                {
                    return;
                }

                // Linear
                RB.Velocity = RB.Velocity + RB.Force * RB.InverseMass * DT;
                RB.Velocity = RB.Velocity * (1.0f - RB.LinearDamping);

                // Angular
                if (!RB.bFreezeRotation)
                {
                    RB.AngularVelocity = RB.AngularVelocity + RB.Torque * RB.InverseInertia * DT;
                    RB.AngularVelocity = RB.AngularVelocity * (1.0f - RB.AngularDamping);
                }
            }
        );
    }

    void IntegratePositions(float DT)
    {
        World->Each<FRigidbody3DComponent, ECS::FTransformComponent>(
            [DT](ECS::FEntityID Entity, FRigidbody3DComponent& RB, ECS::FTransformComponent& Transform)
            {
                if (RB.bIsStatic || RB.bIsSleeping)
                {
                    return;
                }

                Transform.Position = Transform.Position + RB.Velocity * DT;

                if (!RB.bFreezeRotation)
                {
                    float AngularSpeed = RB.AngularVelocity.Length();
                    if (AngularSpeed > 0.0001f)
                    {
                        Math::Vec3 Axis = RB.AngularVelocity / AngularSpeed;
                        float Angle = AngularSpeed * DT;
                        // Apply rotation (simplified)
                        Transform.Rotation += Angle;
                    }
                }
            }
        );
    }

    void BroadPhase()
    {
        // Simple O(n^2) broad phase - would use spatial hash or BVH for real game
        std::vector<std::pair<ECS::FEntityID, FCollider3DComponent*>> Colliders;

        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                // Update world bounds
                Collider.WorldCenter = Transform.Position;
                
                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Sphere.Center;
                        Collider.WorldRadius = Collider.Sphere.Radius;
                        break;
                    case EColliderType::Box:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Box.Center;
                        Collider.WorldRadius = Collider.Box.HalfExtents.Length();
                        break;
                    case EColliderType::Capsule:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Capsule.Center;
                        Collider.WorldRadius = Collider.Capsule.Height * 0.5f + Collider.Capsule.Radius;
                        break;
                    default:
                        break;
                }

                Colliders.push_back({Entity, &Collider});
            }
        );

        // Check pairs
        for (size_t i = 0; i < Colliders.size(); i++)
        {
            for (size_t j = i + 1; j < Colliders.size(); j++)
            {
                auto& [EntityA, ColliderA] = Colliders[i];
                auto& [EntityB, ColliderB] = Colliders[j];

                // Layer mask check
                if (!(ColliderA->Layer & ColliderB->Mask) || !(ColliderB->Layer & ColliderA->Mask))
                {
                    continue;
                }

                // Bounding sphere check
                Math::Vec3 Diff = ColliderB->WorldCenter - ColliderA->WorldCenter;
                float DistSq = Diff.x * Diff.x + Diff.y * Diff.y + Diff.z * Diff.z;
                float RadiusSum = ColliderA->WorldRadius + ColliderB->WorldRadius;

                if (DistSq < RadiusSum * RadiusSum)
                {
                    FCollisionPair Pair;
                    Pair.EntityA = EntityA;
                    Pair.EntityB = EntityB;
                    CollisionPairs.push_back(Pair);
                }
            }
        }
    }

    void NarrowPhase()
    {
        for (auto& Pair : CollisionPairs)
        {
            auto* ColliderA = World->GetComponent<FCollider3DComponent>(Pair.EntityA);
            auto* ColliderB = World->GetComponent<FCollider3DComponent>(Pair.EntityB);
            auto* TransformA = World->GetComponent<ECS::FTransformComponent>(Pair.EntityA);
            auto* TransformB = World->GetComponent<ECS::FTransformComponent>(Pair.EntityB);

            if (!ColliderA || !ColliderB || !TransformA || !TransformB)
            {
                continue;
            }

            FContactPoint Contact;
            bool HasContact = false;

            // Dispatch based on collider types
            if (ColliderA->Type == EColliderType::Sphere && ColliderB->Type == EColliderType::Sphere)
            {
                HasContact = SphereSphereContact(
                    TransformA->Position + ColliderA->Sphere.Center, ColliderA->Sphere.Radius,
                    TransformB->Position + ColliderB->Sphere.Center, ColliderB->Sphere.Radius,
                    Contact);
            }
            else if (ColliderA->Type == EColliderType::Sphere && ColliderB->Type == EColliderType::Box)
            {
                HasContact = SphereBoxContact(
                    TransformA->Position + ColliderA->Sphere.Center, ColliderA->Sphere.Radius,
                    TransformB->Position + ColliderB->Box.Center, ColliderB->Box.HalfExtents,
                    Contact);
            }
            else if (ColliderA->Type == EColliderType::Box && ColliderB->Type == EColliderType::Sphere)
            {
                HasContact = SphereBoxContact(
                    TransformB->Position + ColliderB->Sphere.Center, ColliderB->Sphere.Radius,
                    TransformA->Position + ColliderA->Box.Center, ColliderA->Box.HalfExtents,
                    Contact);
                Contact.Normal = Contact.Normal * -1.0f;
            }
            else if (ColliderA->Type == EColliderType::Box && ColliderB->Type == EColliderType::Box)
            {
                HasContact = BoxBoxContact(
                    TransformA->Position + ColliderA->Box.Center, ColliderA->Box.HalfExtents,
                    TransformB->Position + ColliderB->Box.Center, ColliderB->Box.HalfExtents,
                    Contact);
            }

            if (HasContact)
            {
                Pair.Contacts.push_back(Contact);
            }
        }

        // Remove pairs with no contacts
        CollisionPairs.erase(
            std::remove_if(CollisionPairs.begin(), CollisionPairs.end(),
                [](const FCollisionPair& P) { return P.Contacts.empty(); }),
            CollisionPairs.end()
        );
    }

    void ResolveVelocities()
    {
        for (auto& Pair : CollisionPairs)
        {
            auto* ColliderA = World->GetComponent<FCollider3DComponent>(Pair.EntityA);
            auto* ColliderB = World->GetComponent<FCollider3DComponent>(Pair.EntityB);

            if (ColliderA->bIsTrigger || ColliderB->bIsTrigger)
            {
                continue;
            }

            auto* RbA = World->GetComponent<FRigidbody3DComponent>(Pair.EntityA);
            auto* RbB = World->GetComponent<FRigidbody3DComponent>(Pair.EntityB);

            for (auto& Contact : Pair.Contacts)
            {
                float InvMassA = RbA ? RbA->InverseMass : 0.0f;
                float InvMassB = RbB ? RbB->InverseMass : 0.0f;
                float TotalInvMass = InvMassA + InvMassB;

                if (TotalInvMass < 0.0001f)
                {
                    continue;
                }

                Math::Vec3 VelA = RbA ? RbA->Velocity : Math::Vec3{0,0,0};
                Math::Vec3 VelB = RbB ? RbB->Velocity : Math::Vec3{0,0,0};
                Math::Vec3 RelVel = VelA - VelB;

                float VelAlongNormal = RelVel.Dot(Contact.Normal);

                if (VelAlongNormal > 0)
                {
                    continue; // Moving apart
                }

                // Restitution
                float E = std::min(ColliderA->Bounciness, ColliderB->Bounciness);
                
                // Impulse magnitude
                float J = -(1.0f + E) * VelAlongNormal / TotalInvMass;
                
                Pair.ImpulseMagnitude = std::max(Pair.ImpulseMagnitude, std::abs(J));

                Math::Vec3 Impulse = Contact.Normal * J;

                if (RbA && !RbA->bIsKinematic && !RbA->bIsStatic)
                {
                    RbA->Velocity = RbA->Velocity + Impulse * InvMassA;
                }
                if (RbB && !RbB->bIsKinematic && !RbB->bIsStatic)
                {
                    RbB->Velocity = RbB->Velocity - Impulse * InvMassB;
                }

                // Friction
                Math::Vec3 Tangent = RelVel - Contact.Normal * VelAlongNormal;
                float TangentLen = Tangent.Length();
                
                if (TangentLen > 0.0001f)
                {
                    Tangent = Tangent / TangentLen;
                    
                    float Mu = (ColliderA->Friction + ColliderB->Friction) * 0.5f;
                    float JT = -RelVel.Dot(Tangent) / TotalInvMass;
                    
                    // Clamp friction
                    if (std::abs(JT) < std::abs(J * Mu))
                    {
                        Math::Vec3 FrictionImpulse = Tangent * JT;
                        
                        if (RbA && !RbA->bIsKinematic && !RbA->bIsStatic)
                        {
                            RbA->Velocity = RbA->Velocity + FrictionImpulse * InvMassA;
                        }
                        if (RbB && !RbB->bIsKinematic && !RbB->bIsStatic)
                        {
                            RbB->Velocity = RbB->Velocity - FrictionImpulse * InvMassB;
                        }
                    }
                }
            }
        }
    }

    void ResolvePositions()
    {
        const float Slop = 0.01f;
        const float Percent = 0.8f;

        for (auto& Pair : CollisionPairs)
        {
            auto* ColliderA = World->GetComponent<FCollider3DComponent>(Pair.EntityA);
            auto* ColliderB = World->GetComponent<FCollider3DComponent>(Pair.EntityB);

            if (ColliderA->bIsTrigger || ColliderB->bIsTrigger)
            {
                continue;
            }

            auto* RbA = World->GetComponent<FRigidbody3DComponent>(Pair.EntityA);
            auto* RbB = World->GetComponent<FRigidbody3DComponent>(Pair.EntityB);
            auto* TransformA = World->GetComponent<ECS::FTransformComponent>(Pair.EntityA);
            auto* TransformB = World->GetComponent<ECS::FTransformComponent>(Pair.EntityB);

            for (auto& Contact : Pair.Contacts)
            {
                float InvMassA = (RbA && !RbA->bIsStatic && !RbA->bIsKinematic) ? RbA->InverseMass : 0.0f;
                float InvMassB = (RbB && !RbB->bIsStatic && !RbB->bIsKinematic) ? RbB->InverseMass : 0.0f;
                float TotalInvMass = InvMassA + InvMassB;

                if (TotalInvMass < 0.0001f)
                {
                    continue;
                }

                float Correction = std::max(Contact.Penetration - Slop, 0.0f) * Percent / TotalInvMass;
                Math::Vec3 CorrectionVec = Contact.Normal * Correction;

                if (TransformA && InvMassA > 0)
                {
                    TransformA->Position = TransformA->Position + CorrectionVec * InvMassA;
                }
                if (TransformB && InvMassB > 0)
                {
                    TransformB->Position = TransformB->Position - CorrectionVec * InvMassB;
                }
            }
        }
    }

    void UpdateTransforms()
    {
        // Update collider world positions
        World->Each<FCollider3DComponent, ECS::FTransformComponent>(
            [](ECS::FEntityID Entity, FCollider3DComponent& Collider, ECS::FTransformComponent& Transform)
            {
                Collider.WorldCenter = Transform.Position;
                
                switch (Collider.Type)
                {
                    case EColliderType::Sphere:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Sphere.Center;
                        break;
                    case EColliderType::Box:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Box.Center;
                        break;
                    case EColliderType::Capsule:
                        Collider.WorldCenter = Collider.WorldCenter + Collider.Capsule.Center;
                        break;
                    default:
                        break;
                }
            }
        );
    }

    void FireCollisionCallbacks()
    {
        for (const auto& Pair : CollisionPairs)
        {
            auto* ColliderA = World->GetComponent<FCollider3DComponent>(Pair.EntityA);
            auto* ColliderB = World->GetComponent<FCollider3DComponent>(Pair.EntityB);

            bool IsTrigger = ColliderA->bIsTrigger || ColliderB->bIsTrigger;
            
            // Check if this is a new collision
            bool WasColliding = false;
            for (const auto& Prev : PreviousCollisions)
            {
                if ((Prev.EntityA == Pair.EntityA && Prev.EntityB == Pair.EntityB) ||
                    (Prev.EntityA == Pair.EntityB && Prev.EntityB == Pair.EntityA))
                {
                    WasColliding = true;
                    break;
                }
            }

            if (!WasColliding)
            {
                if (IsTrigger)
                {
                    if (OnTriggerEnter)
                    {
                        OnTriggerEnter(Pair.EntityA, Pair.EntityB);
                    }
                }
                else
                {
                    if (OnCollisionEnter && !Pair.Contacts.empty())
                    {
                        OnCollisionEnter(Pair.EntityA, Pair.EntityB, Pair.Contacts[0]);
                    }
                }
            }
        }

        // Check for collision exits
        for (const auto& Prev : PreviousCollisions)
        {
            bool StillColliding = false;
            for (const auto& Current : CollisionPairs)
            {
                if ((Current.EntityA == Prev.EntityA && Current.EntityB == Prev.EntityB) ||
                    (Current.EntityA == Prev.EntityB && Current.EntityB == Prev.EntityA))
                {
                    StillColliding = true;
                    break;
                }
            }

            if (!StillColliding)
            {
                auto* ColliderA = World->GetComponent<FCollider3DComponent>(Prev.EntityA);
                auto* ColliderB = World->GetComponent<FCollider3DComponent>(Prev.EntityB);
                
                if (ColliderA && ColliderB)
                {
                    bool IsTrigger = ColliderA->bIsTrigger || ColliderB->bIsTrigger;
                    
                    if (IsTrigger)
                    {
                        if (OnTriggerExit)
                        {
                            OnTriggerExit(Prev.EntityA, Prev.EntityB);
                        }
                    }
                    else
                    {
                        if (OnCollisionExit)
                        {
                            OnCollisionExit(Prev.EntityA, Prev.EntityB);
                        }
                    }
                }
            }
        }

        PreviousCollisions = CollisionPairs;
    }

    void ClearForces()
    {
        World->Each<FRigidbody3DComponent>(
            [](ECS::FEntityID Entity, FRigidbody3DComponent& RB)
            {
                RB.Force = Math::Vec3{0, 0, 0};
                RB.Torque = Math::Vec3{0, 0, 0};
            }
        );
    }

    void UpdateSleep(float DT)
    {
        World->Each<FRigidbody3DComponent>(
            [DT](ECS::FEntityID Entity, FRigidbody3DComponent& RB)
            {
                if (RB.bIsStatic)
                {
                    return;
                }

                float Speed = RB.Velocity.Length() + RB.AngularVelocity.Length();
                
                if (Speed < RB.SleepThreshold)
                {
                    RB.SleepTimer += DT;
                    if (RB.SleepTimer > 0.5f)
                    {
                        RB.bIsSleeping = true;
                        RB.Velocity = Math::Vec3{0, 0, 0};
                        RB.AngularVelocity = Math::Vec3{0, 0, 0};
                    }
                }
                else
                {
                    RB.SleepTimer = 0.0f;
                    RB.bIsSleeping = false;
                }
            }
        );
    }

    void InterpolateTransforms(float Alpha)
    {
        // Would interpolate between previous and current state for smooth rendering
        // For now, just using current state
    }

    // ========================================================================
    // Raycast Helpers
    // ========================================================================

    FRaycastHit RaycastSphere(const Math::Vec3& Origin, const Math::Vec3& Dir,
                              const Math::Vec3& Center, float Radius)
    {
        FRaycastHit Hit;
        
        Math::Vec3 OC = Origin - Center;
        float A = Dir.Dot(Dir);
        float B = 2.0f * OC.Dot(Dir);
        float C = OC.Dot(OC) - Radius * Radius;
        float Discriminant = B * B - 4 * A * C;

        if (Discriminant < 0)
        {
            return Hit;
        }

        float T = (-B - std::sqrt(Discriminant)) / (2.0f * A);
        
        if (T < 0)
        {
            T = (-B + std::sqrt(Discriminant)) / (2.0f * A);
        }

        if (T < 0)
        {
            return Hit;
        }

        Hit.bHit = true;
        Hit.Distance = T;
        Hit.Point = Origin + Dir * T;
        Hit.Normal = (Hit.Point - Center).Normalized();

        return Hit;
    }

    FRaycastHit RaycastBox(const Math::Vec3& Origin, const Math::Vec3& Dir,
                           const Math::Vec3& Center, const Math::Vec3& HalfExtents,
                           const Math::Quaternion& Rotation)
    {
        FRaycastHit Hit;

        // Transform ray to box local space (simplified - no rotation)
        Math::Vec3 Min = Center - HalfExtents;
        Math::Vec3 Max = Center + HalfExtents;

        float TMin = 0.0f;
        float TMax = 1000000.0f;
        int HitAxis = -1;
        bool HitSideMin = false;

        for (int i = 0; i < 3; i++)
        {
            float O = (i == 0) ? Origin.x : (i == 1) ? Origin.y : Origin.z;
            float D = (i == 0) ? Dir.x : (i == 1) ? Dir.y : Dir.z;
            float MinV = (i == 0) ? Min.x : (i == 1) ? Min.y : Min.z;
            float MaxV = (i == 0) ? Max.x : (i == 1) ? Max.y : Max.z;

            if (std::abs(D) < 0.0001f)
            {
                if (O < MinV || O > MaxV)
                {
                    return Hit;
                }
            }
            else
            {
                float InvD = 1.0f / D;
                float T1 = (MinV - O) * InvD;
                float T2 = (MaxV - O) * InvD;

                bool Side = (T1 < T2);
                if (T1 > T2) std::swap(T1, T2);

                if (T1 > TMin)
                {
                    TMin = T1;
                    HitAxis = i;
                    HitSideMin = Side;
                }
                if (T2 < TMax) TMax = T2;

                if (TMin > TMax)
                {
                    return Hit;
                }
            }
        }

        if (TMin < 0)
        {
            return Hit;
        }

        Hit.bHit = true;
        Hit.Distance = TMin;
        Hit.Point = Origin + Dir * TMin;

        Hit.Normal = Math::Vec3{0, 0, 0};
        if (HitAxis == 0) Hit.Normal.x = HitSideMin ? -1.0f : 1.0f;
        else if (HitAxis == 1) Hit.Normal.y = HitSideMin ? -1.0f : 1.0f;
        else if (HitAxis == 2) Hit.Normal.z = HitSideMin ? -1.0f : 1.0f;

        return Hit;
    }

    FRaycastHit RaycastCapsule(const Math::Vec3& Origin, const Math::Vec3& Dir,
                               const Math::Vec3& Center, float Radius, float Height, int Axis)
    {
        // Simplified capsule raycast - treat as sphere
        FRaycastHit Hit = RaycastSphere(Origin, Dir, Center, Height * 0.5f + Radius);
        return Hit;
    }

    // ========================================================================
    // Collision Test Helpers
    // ========================================================================

    bool SphereSphereTest(const Math::Vec3& A, float RA, const Math::Vec3& B, float RB)
    {
        Math::Vec3 Diff = B - A;
        float DistSq = Diff.x * Diff.x + Diff.y * Diff.y + Diff.z * Diff.z;
        float RadiusSum = RA + RB;
        return DistSq < RadiusSum * RadiusSum;
    }

    bool SphereBoxTest(const Math::Vec3& SphereCenter, float Radius,
                       const Math::Vec3& BoxCenter, const Math::Vec3& HalfExtents)
    {
        Math::Vec3 Closest{
            std::max(BoxCenter.x - HalfExtents.x, std::min(SphereCenter.x, BoxCenter.x + HalfExtents.x)),
            std::max(BoxCenter.y - HalfExtents.y, std::min(SphereCenter.y, BoxCenter.y + HalfExtents.y)),
            std::max(BoxCenter.z - HalfExtents.z, std::min(SphereCenter.z, BoxCenter.z + HalfExtents.z))
        };

        Math::Vec3 Diff = SphereCenter - Closest;
        float DistSq = Diff.x * Diff.x + Diff.y * Diff.y + Diff.z * Diff.z;
        return DistSq < Radius * Radius;
    }

    bool BoxBoxTest(const Math::Vec3& A, const Math::Vec3& HalfA,
                    const Math::Vec3& B, const Math::Vec3& HalfB)
    {
        return std::abs(A.x - B.x) < (HalfA.x + HalfB.x) &&
               std::abs(A.y - B.y) < (HalfA.y + HalfB.y) &&
               std::abs(A.z - B.z) < (HalfA.z + HalfB.z);
    }

    bool SphereCapsuleTest(const Math::Vec3& SphereCenter, float SphereRadius,
                           const Math::Vec3& CapsuleCenter, float CapsuleRadius, 
                           float CapsuleHeight, int Direction)
    {
        // Simplified - treat capsule as sphere
        float TotalRadius = SphereRadius + CapsuleRadius + CapsuleHeight * 0.5f;
        Math::Vec3 Diff = SphereCenter - CapsuleCenter;
        float DistSq = Diff.x * Diff.x + Diff.y * Diff.y + Diff.z * Diff.z;
        return DistSq < TotalRadius * TotalRadius;
    }

    // ========================================================================
    // Contact Generation
    // ========================================================================

    bool SphereSphereContact(const Math::Vec3& A, float RA, const Math::Vec3& B, float RB,
                             FContactPoint& Contact)
    {
        Math::Vec3 Diff = B - A;
        float Dist = Diff.Length();
        float RadiusSum = RA + RB;

        if (Dist >= RadiusSum)
        {
            return false;
        }

        if (Dist > 0.0001f)
        {
            Contact.Normal = Diff / Dist;
        }
        else
        {
            Contact.Normal = Math::Vec3{0, 1, 0};
        }

        Contact.Penetration = RadiusSum - Dist;
        Contact.Point = A + Contact.Normal * RA;

        return true;
    }

    bool SphereBoxContact(const Math::Vec3& SphereCenter, float Radius,
                          const Math::Vec3& BoxCenter, const Math::Vec3& HalfExtents,
                          FContactPoint& Contact)
    {
        Math::Vec3 Closest{
            std::max(BoxCenter.x - HalfExtents.x, std::min(SphereCenter.x, BoxCenter.x + HalfExtents.x)),
            std::max(BoxCenter.y - HalfExtents.y, std::min(SphereCenter.y, BoxCenter.y + HalfExtents.y)),
            std::max(BoxCenter.z - HalfExtents.z, std::min(SphereCenter.z, BoxCenter.z + HalfExtents.z))
        };

        Math::Vec3 Diff = SphereCenter - Closest;
        float Dist = Diff.Length();

        if (Dist >= Radius)
        {
            return false;
        }

        if (Dist > 0.0001f)
        {
            Contact.Normal = Diff / Dist;
        }
        else
        {
            // Sphere center is inside box
            Contact.Normal = Math::Vec3{0, 1, 0};
        }

        Contact.Penetration = Radius - Dist;
        Contact.Point = Closest;

        return true;
    }

    bool BoxBoxContact(const Math::Vec3& A, const Math::Vec3& HalfA,
                       const Math::Vec3& B, const Math::Vec3& HalfB,
                       FContactPoint& Contact)
    {
        Math::Vec3 Diff = B - A;
        
        float OverlapX = (HalfA.x + HalfB.x) - std::abs(Diff.x);
        float OverlapY = (HalfA.y + HalfB.y) - std::abs(Diff.y);
        float OverlapZ = (HalfA.z + HalfB.z) - std::abs(Diff.z);

        if (OverlapX <= 0 || OverlapY <= 0 || OverlapZ <= 0)
        {
            return false;
        }

        // Find minimum overlap axis
        if (OverlapX < OverlapY && OverlapX < OverlapZ)
        {
            Contact.Penetration = OverlapX;
            Contact.Normal = Math::Vec3{Diff.x > 0 ? 1.0f : -1.0f, 0, 0};
        }
        else if (OverlapY < OverlapZ)
        {
            Contact.Penetration = OverlapY;
            Contact.Normal = Math::Vec3{0, Diff.y > 0 ? 1.0f : -1.0f, 0};
        }
        else
        {
            Contact.Penetration = OverlapZ;
            Contact.Normal = Math::Vec3{0, 0, Diff.z > 0 ? 1.0f : -1.0f};
        }

        // Contact point (simplified)
        Contact.Point = A + Contact.Normal * (HalfA.x + HalfA.y + HalfA.z) / 3.0f;

        return true;
    }
};

// ============================================================================
// Physics System for ECS
// ============================================================================

class FPhysics3DSystem : public ECS::ISystem
{
public:
    FPhysicsWorld PhysicsWorld;

    void Init(ECS::FWorld& World) override
    {
        PhysicsWorld.Initialize(&World);
    }

    void Update(ECS::FWorld& World, float DeltaTime) override
    {
        PhysicsWorld.Step(DeltaTime);
    }

    int GetPriority() const override { return 100; }
};

} // namespace Titan::Physics

#endif // TITAN_PHYSICS_3D_HPP


