#pragma once

#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/Rigidbody.hpp”
#include “PhysicsWorld.hpp”

namespace Titan::Physics {

using namespace ECS::Components;

class CharacterController {
public:
CharacterController(float radius = 0.5f, float height = 2.0f)
: radius(radius), height(height) {

```
    slopeLimit = 45.0f;
    stepOffset = 0.3f;
    skinWidth = 0.08f;
    minMoveDistance = 0.001f;
    centerOffset = Vec3(0, height * 0.5f, 0);
}

void Move(Vec3& position, const Vec3& motion) {
    if (motion.LengthSquared() < minMoveDistance * minMoveDistance) {
        return;
    }
    
    Vec3 moveAmount = motion;
    
    isGrounded = CheckGrounded(position);
    
    if (isGrounded) {
        Vec3 groundNormal = GetGroundNormal(position);
        
        float slopeAngle = std::acos(groundNormal.Dot(Vec3::Up())) * 180.0f / 3.14159265f;
        
        if (slopeAngle < slopeLimit) {
            Vec3 slopeDir = Vec3::Up().Cross(groundNormal.Cross(Vec3::Up())).Normalized();
            float alignment = moveAmount.Normalized().Dot(slopeDir);
            
            if (std::abs(alignment) > 0.1f) {
                moveAmount = slopeDir * moveAmount.Length() * alignment;
            }
        }
    }
    
    Vec3 finalPosition = position + moveAmount;
    
    auto hit = PhysicsWorld::Get().Raycast(
        finalPosition + centerOffset,
        Vec3::Down(),
        height * 0.5f + skinWidth
    );
    
    if (hit.hit && hit.distance < height * 0.5f + stepOffset) {
        finalPosition.y = hit.point.y;
    }
    
    position = finalPosition;
}

void Jump(Vec3& velocity, float jumpForce) {
    if (isGrounded) {
        velocity.y = jumpForce;
    }
}

bool IsGrounded() const { return isGrounded; }

void SetSlopeLimit(float degrees) { slopeLimit = degrees; }
void SetStepOffset(float offset) { stepOffset = offset; }

float GetRadius() const { return radius; }
float GetHeight() const { return height; }
```

private:
float radius;
float height;
float slopeLimit;
float stepOffset;
float skinWidth;
float minMoveDistance;
Vec3 centerOffset;

```
bool isGrounded;

bool CheckGrounded(const Vec3& position) {
    auto hit = PhysicsWorld::Get().Raycast(
        position + centerOffset,
        Vec3::Down(),
        height * 0.5f + skinWidth + 0.1f
    );
    
    return hit.hit && hit.distance <= height * 0.5f + skinWidth + 0.05f;
}

Vec3 GetGroundNormal(const Vec3& position) {
    auto hit = PhysicsWorld::Get().Raycast(
        position + centerOffset,
        Vec3::Down(),
        height * 0.5f + skinWidth + 0.1f
    );
    
    if (hit.hit) {
        return hit.normal;
    }
    
    return Vec3::Up();
}
```

};

struct FirstPersonController {
float moveSpeed = 5.0f;
float sprintSpeed = 8.0f;
float jumpForce = 5.0f;
float mouseSensitivity = 0.1f;

```
float pitch = 0.0f;
float yaw = 0.0f;

CharacterController character;

FirstPersonController() : character(0.4f, 1.8f) {}

void Update(Transform& transform, Rigidbody& rb, float dt, 
            const Vec3& moveInput, const Vec2& mouseInput, bool jump, bool sprint) {
    
    yaw += mouseInput.x * mouseSensitivity;
    pitch -= mouseInput.y * mouseSensitivity;
    pitch = std::max(-89.0f, std::min(89.0f, pitch));
    
    float pitchRad = pitch * 3.14159265f / 180.0f;
    float yawRad = yaw * 3.14159265f / 180.0f;
    
    transform.rotation = Quaternion::FromEuler(pitchRad, yawRad, 0);
    
    Vec3 forward = transform.Forward();
    Vec3 right = transform.Right();
    
    forward.y = 0;
    forward = forward.Normalized();
    right.y = 0;
    right = right.Normalized();
    
    Vec3 moveDir = forward * moveInput.z + right * moveInput.x;
    if (moveDir.LengthSquared() > 0.001f) {
        moveDir = moveDir.Normalized();
    }
    
    float speed = sprint ? sprintSpeed : moveSpeed;
    Vec3 motion = moveDir * speed * dt;
    
    character.Move(transform.position, motion);
    
    if (jump) {
        character.Jump(rb.velocity, jumpForce);
    }
    
    if (!character.IsGrounded()) {
        rb.velocity.y += PhysicsWorld::Get().GetGravity().y * dt;
    } else {
        if (rb.velocity.y < 0) {
            rb.velocity.y = 0;
        }
    }
    
    transform.position.y += rb.velocity.y * dt;
}
```

};

}
