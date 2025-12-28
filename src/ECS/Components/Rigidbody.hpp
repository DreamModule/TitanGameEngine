#pragma once

#include “Transform.hpp”

namespace Titan::ECS::Components {

enum class RigidbodyType {
Static,
Kinematic,
Dynamic
};

struct Rigidbody {
RigidbodyType type = RigidbodyType::Dynamic;

```
float mass = 1.0f;
float drag = 0.05f;
float angularDrag = 0.05f;

Vec3 velocity = Vec3::Zero();
Vec3 angularVelocity = Vec3::Zero();

Vec3 force = Vec3::Zero();
Vec3 torque = Vec3::Zero();

bool useGravity = true;
bool isKinematic = false;
bool freezeRotation = false;

Vec3 constraints = Vec3::Zero();

void* physicsBody = nullptr;

Rigidbody() = default;

Rigidbody(RigidbodyType type, float mass = 1.0f) 
    : type(type), mass(mass) {}

void AddForce(const Vec3& force) {
    this->force += force;
}

void AddImpulse(const Vec3& impulse) {
    if (mass > 0.0f) {
        velocity += impulse / mass;
    }
}

void AddTorque(const Vec3& torque) {
    this->torque += torque;
}

void SetVelocity(const Vec3& vel) {
    velocity = vel;
}

Vec3 GetVelocity() const {
    return velocity;
}
```

};

enum class ColliderType {
Box,
Sphere,
Capsule,
Mesh
};

struct Collider {
ColliderType type = ColliderType::Box;

```
Vec3 center = Vec3::Zero();
Vec3 size = Vec3::One();

float radius = 0.5f;
float height = 2.0f;

bool isTrigger = false;

int layer = 0;
int layerMask = -1;

void* physicsShape = nullptr;

Collider() = default;

static Collider Box(const Vec3& size) {
    Collider c;
    c.type = ColliderType::Box;
    c.size = size;
    return c;
}

static Collider Sphere(float radius) {
    Collider c;
    c.type = ColliderType::Sphere;
    c.radius = radius;
    return c;
}

static Collider Capsule(float radius, float height) {
    Collider c;
    c.type = ColliderType::Capsule;
    c.radius = radius;
    c.height = height;
    return c;
}
```

};

struct CollisionInfo {
ECS::Entity otherEntity;
Vec3 point;
Vec3 normal;
float penetration;
Vec3 relativeVelocity;
};

}
