#pragma once

#include “../ECS/World.hpp”
#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/Rigidbody.hpp”
#include <vector>
#include <functional>

namespace Titan::Physics {

using namespace ECS;
using namespace ECS::Components;

struct RaycastHit {
bool hit;
Entity entity;
Vec3 point;
Vec3 normal;
float distance;
};

struct PhysicsSettings {
Vec3 gravity = Vec3(0, -9.81f, 0);
float fixedTimestep = 1.0f / 60.0f;
int solverIterations = 8;
float sleepThreshold = 0.005f;
float defaultContactOffset = 0.01f;
bool enableCCD = false;
};

class PhysicsWorld {
public:
static PhysicsWorld& Get() {
static PhysicsWorld instance;
return instance;
}

```
void Init(const PhysicsSettings& settings = PhysicsSettings()) {
    this->settings = settings;
    initialized = true;
}

void Shutdown() {
    initialized = false;
}

void SetWorld(World* world) {
    this->world = world;
}

void Step(float deltaTime) {
    if (!initialized || !world) return;
    
    accumulator += deltaTime;
    
    while (accumulator >= settings.fixedTimestep) {
        FixedStep(settings.fixedTimestep);
        accumulator -= settings.fixedTimestep;
    }
}

void FixedStep(float dt) {
    ApplyGravity(dt);
    IntegrateVelocities(dt);
    DetectCollisions();
    ResolveCollisions(dt);
    IntegratePositions(dt);
    UpdateTransforms();
}

RaycastHit Raycast(const Vec3& origin, const Vec3& direction, float maxDistance = 1000.0f) {
    RaycastHit result;
    result.hit = false;
    result.distance = maxDistance;
    
    if (!world) return result;
    
    Vec3 rayEnd = origin + direction.Normalized() * maxDistance;
    
    world->Each<Transform, Collider>([&](Entity e, Transform& t, Collider& col) {
        if (col.isTrigger) return;
        
        Vec3 colliderPos = t.position + col.center;
        
        if (col.type == ColliderType::Sphere) {
            Vec3 oc = origin - colliderPos;
            float a = direction.Dot(direction);
            float b = 2.0f * oc.Dot(direction);
            float c = oc.Dot(oc) - col.radius * col.radius;
            float discriminant = b * b - 4 * a * c;
            
            if (discriminant >= 0) {
                float dist = (-b - std::sqrt(discriminant)) / (2.0f * a);
                if (dist > 0 && dist < result.distance) {
                    result.hit = true;
                    result.entity = e;
                    result.distance = dist;
                    result.point = origin + direction.Normalized() * dist;
                    result.normal = (result.point - colliderPos).Normalized();
                }
            }
        }
    });
    
    return result;
}

bool OverlapSphere(const Vec3& position, float radius, std::vector<Entity>& outEntities) {
    if (!world) return false;
    
    bool foundAny = false;
    
    world->Each<Transform, Collider>([&](Entity e, Transform& t, Collider& col) {
        Vec3 colliderPos = t.position + col.center;
        float dist = (colliderPos - position).Length();
        
        float checkRadius = radius;
        if (col.type == ColliderType::Sphere) {
            checkRadius += col.radius;
        }
        
        if (dist < checkRadius) {
            outEntities.push_back(e);
            foundAny = true;
        }
    });
    
    return foundAny;
}

const PhysicsSettings& GetSettings() const { return settings; }
void SetGravity(const Vec3& gravity) { settings.gravity = gravity; }
Vec3 GetGravity() const { return settings.gravity; }
```

private:
PhysicsWorld() : world(nullptr), initialized(false), accumulator(0.0f) {}

```
World* world;
PhysicsSettings settings;
bool initialized;
float accumulator;

struct Contact {
    Entity entityA;
    Entity entityB;
    Vec3 point;
    Vec3 normal;
    float penetration;
};

std::vector<Contact> contacts;

void ApplyGravity(float dt) {
    world->Each<Rigidbody>([this, dt](Entity e, Rigidbody& rb) {
        if (rb.type != RigidbodyType::Dynamic) return;
        if (!rb.useGravity) return;
        
        rb.force += settings.gravity * rb.mass;
    });
}

void IntegrateVelocities(float dt) {
    world->Each<Rigidbody>([dt](Entity e, Rigidbody& rb) {
        if (rb.type != RigidbodyType::Dynamic) return;
        
        if (rb.mass > 0.0f) {
            Vec3 acceleration = rb.force / rb.mass;
            rb.velocity += acceleration * dt;
        }
        
        rb.velocity *= (1.0f - rb.drag * dt);
        rb.angularVelocity *= (1.0f - rb.angularDrag * dt);
        
        rb.force = Vec3::Zero();
        rb.torque = Vec3::Zero();
    });
}

void DetectCollisions() {
    contacts.clear();
    
    std::vector<Entity> entities;
    world->Each<Transform, Collider>([&entities](Entity e, Transform& t, Collider& c) {
        entities.push_back(e);
    });
    
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            Entity eA = entities[i];
            Entity eB = entities[j];
            
            if (!world->HasComponent<Transform>(eA) || !world->HasComponent<Transform>(eB)) continue;
            if (!world->HasComponent<Collider>(eA) || !world->HasComponent<Collider>(eB)) continue;
            
            auto& tA = world->GetComponent<Transform>(eA);
            auto& tB = world->GetComponent<Transform>(eB);
            auto& cA = world->GetComponent<Collider>(eA);
            auto& cB = world->GetComponent<Collider>(eB);
            
            if (cA.isTrigger || cB.isTrigger) continue;
            
            Contact contact;
            if (CheckCollision(eA, eB, tA, tB, cA, cB, contact)) {
                contacts.push_back(contact);
            }
        }
    }
}

bool CheckCollision(Entity eA, Entity eB, 
                   const Transform& tA, const Transform& tB,
                   const Collider& cA, const Collider& cB,
                   Contact& outContact) {
    
    if (cA.type == ColliderType::Sphere && cB.type == ColliderType::Sphere) {
        Vec3 posA = tA.position + cA.center;
        Vec3 posB = tB.position + cB.center;
        
        Vec3 delta = posB - posA;
        float dist = delta.Length();
        float minDist = cA.radius + cB.radius;
        
        if (dist < minDist) {
            outContact.entityA = eA;
            outContact.entityB = eB;
            outContact.normal = dist > 0.0001f ? delta / dist : Vec3::Up();
            outContact.penetration = minDist - dist;
            outContact.point = posA + outContact.normal * cA.radius;
            return true;
        }
    }
    
    if (cA.type == ColliderType::Box && cB.type == ColliderType::Box) {
        Vec3 posA = tA.position + cA.center;
        Vec3 posB = tB.position + cB.center;
        
        Vec3 halfA = cA.size * 0.5f;
        Vec3 halfB = cB.size * 0.5f;
        
        Vec3 delta = posB - posA;
        
        float overlapX = (halfA.x + halfB.x) - std::abs(delta.x);
        float overlapY = (halfA.y + halfB.y) - std::abs(delta.y);
        float overlapZ = (halfA.z + halfB.z) - std::abs(delta.z);
        
        if (overlapX > 0 && overlapY > 0 && overlapZ > 0) {
            outContact.entityA = eA;
            outContact.entityB = eB;
            
            if (overlapX < overlapY && overlapX < overlapZ) {
                outContact.penetration = overlapX;
                outContact.normal = delta.x > 0 ? Vec3::Right() : Vec3::Left();
            } else if (overlapY < overlapZ) {
                outContact.penetration = overlapY;
                outContact.normal = delta.y > 0 ? Vec3::Up() : Vec3::Down();
            } else {
                outContact.penetration = overlapZ;
                outContact.normal = delta.z > 0 ? Vec3::Forward() : Vec3::Back();
            }
            
            outContact.point = posA + outContact.normal * overlapX * 0.5f;
            return true;
        }
    }
    
    return false;
}

void ResolveCollisions(float dt) {
    for (auto& contact : contacts) {
        Entity eA = contact.entityA;
        Entity eB = contact.entityB;
        
        bool hasRbA = world->HasComponent<Rigidbody>(eA);
        bool hasRbB = world->HasComponent<Rigidbody>(eB);
        
        if (!hasRbA && !hasRbB) continue;
        
        Rigidbody* rbA = hasRbA ? &world->GetComponent<Rigidbody>(eA) : nullptr;
        Rigidbody* rbB = hasRbB ? &world->GetComponent<Rigidbody>(eB) : nullptr;
        
        if (rbA && rbA->type == RigidbodyType::Static) rbA = nullptr;
        if (rbB && rbB->type == RigidbodyType::Static) rbB = nullptr;
        
        if (!rbA && !rbB) continue;
        
        Vec3 relativeVel = Vec3::Zero();
        if (rbA) relativeVel += rbA->velocity;
        if (rbB) relativeVel -= rbB->velocity;
        
        float velAlongNormal = relativeVel.Dot(contact.normal);
        
        if (velAlongNormal > 0) continue;
        
        float restitution = 0.3f;
        
        float invMassA = rbA ? (1.0f / rbA->mass) : 0.0f;
        float invMassB = rbB ? (1.0f / rbB->mass) : 0.0f;
        
        float j = -(1.0f + restitution) * velAlongNormal;
        j /= invMassA + invMassB;
        
        Vec3 impulse = contact.normal * j;
        
        if (rbA) {
            rbA->velocity += impulse * invMassA;
        }
        if (rbB) {
            rbB->velocity -= impulse * invMassB;
        }
        
        const float percent = 0.8f;
        const float slop = 0.01f;
        float correctionMag = std::max(contact.penetration - slop, 0.0f) / (invMassA + invMassB) * percent;
        Vec3 correction = contact.normal * correctionMag;
        
        if (rbA && world->HasComponent<Transform>(eA)) {
            auto& tA = world->GetComponent<Transform>(eA);
            tA.position += correction * invMassA;
        }
        if (rbB && world->HasComponent<Transform>(eB)) {
            auto& tB = world->GetComponent<Transform>(eB);
            tB.position -= correction * invMassB;
        }
    }
}

void IntegratePositions(float dt) {
    world->Each<Transform, Rigidbody>([dt](Entity e, Transform& t, Rigidbody& rb) {
        if (rb.type == RigidbodyType::Static) return;
        
        t.position += rb.velocity * dt;
        
        if (!rb.freezeRotation && rb.angularVelocity.LengthSquared() > 0.0001f) {
            Vec3 axis = rb.angularVelocity.Normalized();
            float angle = rb.angularVelocity.Length() * dt;
            Quaternion deltaRot = Quaternion::FromEuler(
                axis.x * angle,
                axis.y * angle,
                axis.z * angle
            );
            t.rotation = t.rotation * deltaRot;
        }
    });
}

void UpdateTransforms() {
}
```

};

}
