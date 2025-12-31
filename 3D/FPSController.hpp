/**
 * Titan FPS Controller System
 * 
 * Full-featured first person controller with:
 * - WASD movement
 * - Mouse look
 * - Jumping, crouching, sprinting
 * - Ground detection
 * - Smooth movement
 */

#ifndef TITAN_FPS_CONTROLLER_HPP
#define TITAN_FPS_CONTROLLER_HPP

#include "Camera3D.hpp"
#include "../Titan_ECS.hpp"
#include "../Titan_Input.hpp"
#include "../Titan_Platform.hpp"

namespace Titan {

// ============================================================================
// FPS Controller System
// ============================================================================

class FFPSControllerSystem : public ECS::ISystem
{
public:
    // Input bindings
    const char* MoveForward = "MoveForward";
    const char* MoveBack = "MoveBack";
    const char* MoveLeft = "MoveLeft";
    const char* MoveRight = "MoveRight";
    const char* Jump = "Jump";
    const char* Sprint = "Sprint";
    const char* Crouch = "Crouch";

    // Mouse state
    bool bFirstMouse = true;
    float LastMouseX = 0.0f;
    float LastMouseY = 0.0f;
    bool bMouseLocked = true;

    void Init(ECS::FWorld& World) override
    {
        // Setup default input bindings
        Input::Manager::MapAction(MoveForward, KeyCode::W);
        Input::Manager::MapAction(MoveBack, KeyCode::S);
        Input::Manager::MapAction(MoveLeft, KeyCode::A);
        Input::Manager::MapAction(MoveRight, KeyCode::D);
        Input::Manager::MapAction(Jump, KeyCode::Space);
        Input::Manager::MapAction(Sprint, KeyCode::LeftShift);
        Input::Manager::MapAction(Crouch, KeyCode::LeftCtrl);

        // Lock cursor
        Platform::SetCursorLocked(true);
        Platform::SetCursorVisible(false);
    }

    void Update(ECS::FWorld& World, float DeltaTime) override
    {
        World.Each<FFPSControllerComponent, ECS::FTransformComponent>(
            [this, DeltaTime](ECS::FEntityID Entity, 
                              FFPSControllerComponent& Controller,
                              ECS::FTransformComponent& Transform)
            {
                if (!Controller.Camera)
                {
                    return;
                }

                // Handle mouse look
                UpdateMouseLook(Controller, DeltaTime);

                // Handle movement
                UpdateMovement(Controller, Transform, DeltaTime);

                // Update camera position to follow player
                UpdateCameraPosition(Controller, Transform);
            }
        );
    }

    int GetPriority() const override { return 10; }

private:
    void UpdateMouseLook(FFPSControllerComponent& Controller, float DeltaTime)
    {
        if (!bMouseLocked)
        {
            return;
        }

        Math::Vec2 MousePos = Platform::GetMousePos();
        
        if (bFirstMouse)
        {
            LastMouseX = MousePos.x;
            LastMouseY = MousePos.y;
            bFirstMouse = false;
            return;
        }

        float DeltaX = MousePos.x - LastMouseX;
        float DeltaY = MousePos.y - LastMouseY;
        
        LastMouseX = MousePos.x;
        LastMouseY = MousePos.y;

        if (Controller.bInvertY)
        {
            DeltaY = -DeltaY;
        }

        Controller.Camera->Rotate(
            DeltaX * Controller.MouseSensitivity,
            DeltaY * Controller.MouseSensitivity
        );

        // Reset mouse to center (for continuous rotation)
        // This would be done by platform layer in a real implementation
    }

    void UpdateMovement(FFPSControllerComponent& Controller, 
                        ECS::FTransformComponent& Transform, 
                        float DeltaTime)
    {
        // Get input direction
        Math::Vec3 InputDir{0, 0, 0};
        
        if (Input::Manager::GetAction(MoveForward)) InputDir.z += 1.0f;
        if (Input::Manager::GetAction(MoveBack))    InputDir.z -= 1.0f;
        if (Input::Manager::GetAction(MoveLeft))    InputDir.x -= 1.0f;
        if (Input::Manager::GetAction(MoveRight))   InputDir.x += 1.0f;

        // Normalize if moving diagonally
        float InputLength = std::sqrt(InputDir.x * InputDir.x + InputDir.z * InputDir.z);
        if (InputLength > 1.0f)
        {
            InputDir.x /= InputLength;
            InputDir.z /= InputLength;
        }

        // Sprint
        Controller.bIsSprinting = Input::Manager::GetAction(Sprint) && !Controller.bIsCrouching;
        
        // Crouch
        if (Input::Manager::GetActionDown(Crouch))
        {
            Controller.bIsCrouching = !Controller.bIsCrouching;
        }

        // Calculate speed
        float Speed = Controller.bIsSprinting ? Controller.SprintSpeed : Controller.MoveSpeed;
        if (Controller.bIsCrouching)
        {
            Speed *= 0.5f;
        }

        // Get camera forward/right for movement direction
        Math::Vec3 CamForward = Controller.Camera->GetForward();
        Math::Vec3 CamRight = Controller.Camera->GetRight();
        
        // Flatten for ground movement
        CamForward.y = 0;
        CamForward = CamForward.Normalized();
        CamRight.y = 0;
        CamRight = CamRight.Normalized();

        // Calculate world-space movement direction
        Math::Vec3 MoveDir = CamForward * InputDir.z + CamRight * InputDir.x;

        // Apply movement
        float ControlMultiplier = Controller.bIsGrounded ? 1.0f : Controller.AirControl;
        
        // Horizontal velocity
        Math::Vec3 HorizontalVelocity{Controller.Velocity.x, 0, Controller.Velocity.z};
        Math::Vec3 TargetVelocity = MoveDir * Speed;
        
        // Interpolate horizontal velocity
        float Friction = Controller.bIsGrounded ? Controller.Friction : Controller.Friction * 0.1f;
        HorizontalVelocity.x = ApproachValue(HorizontalVelocity.x, TargetVelocity.x, Friction * ControlMultiplier * DeltaTime);
        HorizontalVelocity.z = ApproachValue(HorizontalVelocity.z, TargetVelocity.z, Friction * ControlMultiplier * DeltaTime);

        Controller.Velocity.x = HorizontalVelocity.x;
        Controller.Velocity.z = HorizontalVelocity.z;

        // Gravity
        if (!Controller.bIsGrounded)
        {
            Controller.Velocity.y -= Controller.Gravity * DeltaTime;
        }
        else
        {
            Controller.Velocity.y = 0;
        }

        // Jump
        if (Input::Manager::GetActionDown(Jump) && Controller.bIsGrounded)
        {
            Controller.Velocity.y = Controller.JumpForce;
            Controller.bIsGrounded = false;
        }

        // Apply velocity to position
        Transform.Position.x += Controller.Velocity.x * DeltaTime;
        Transform.Position.y += Controller.Velocity.y * DeltaTime;
        Transform.Position.z += Controller.Velocity.z * DeltaTime;

        // Ground check (simple floor at y=0 for now)
        float GroundY = 0.0f;
        float PlayerHeight = Controller.bIsCrouching ? Controller.CrouchingHeight : Controller.StandingHeight;
        float HalfHeight = PlayerHeight * 0.5f;

        if (Transform.Position.y - HalfHeight <= GroundY)
        {
            Transform.Position.y = GroundY + HalfHeight;
            Controller.Velocity.y = 0;
            Controller.bIsGrounded = true;
        }
        else
        {
            Controller.bIsGrounded = false;
        }

        // Footstep sounds
        if (Controller.bIsGrounded && InputLength > 0.1f)
        {
            Controller.FootstepTimer += DeltaTime;
            float Interval = Controller.bIsSprinting ? Controller.FootstepInterval * 0.6f : Controller.FootstepInterval;
            
            if (Controller.FootstepTimer >= Interval)
            {
                Controller.FootstepTimer = 0;
                // TODO: Play footstep sound
            }
        }
    }

    void UpdateCameraPosition(FFPSControllerComponent& Controller, 
                              const ECS::FTransformComponent& Transform)
    {
        float PlayerHeight = Controller.bIsCrouching ? Controller.CrouchingHeight : Controller.StandingHeight;
        float EyeOffset = PlayerHeight * 0.5f - 0.1f; // Eyes near top of player

        Controller.Camera->Position = Math::Vec3{
            Transform.Position.x,
            Transform.Position.y + EyeOffset,
            Transform.Position.z
        };
    }

    float ApproachValue(float Current, float Target, float MaxDelta)
    {
        float Diff = Target - Current;
        if (std::abs(Diff) <= MaxDelta)
        {
            return Target;
        }
        return Current + (Diff > 0 ? MaxDelta : -MaxDelta);
    }
};

// ============================================================================
// Third Person Controller Component
// ============================================================================

struct FThirdPersonControllerComponent
{
    // Movement
    float MoveSpeed = 5.0f;
    float SprintSpeed = 10.0f;
    float JumpForce = 10.0f;
    float Gravity = 25.0f;
    float TurnSpeed = 10.0f;
    
    // Camera
    float CameraDistance = 4.0f;
    float CameraHeight = 1.5f;
    float CameraMinDistance = 1.0f;
    float CameraMaxDistance = 10.0f;
    float CameraSensitivity = 0.2f;
    
    // State
    Math::Vec3 Velocity{0, 0, 0};
    float TargetYaw = 0.0f;
    float CurrentYaw = 0.0f;
    bool bIsGrounded = false;
    bool bIsSprinting = false;
    
    // Camera reference
    FCamera3D* Camera = nullptr;
};

// ============================================================================
// Third Person Controller System
// ============================================================================

class FThirdPersonControllerSystem : public ECS::ISystem
{
public:
    void Update(ECS::FWorld& World, float DeltaTime) override
    {
        World.Each<FThirdPersonControllerComponent, ECS::FTransformComponent>(
            [this, DeltaTime](ECS::FEntityID Entity,
                              FThirdPersonControllerComponent& Controller,
                              ECS::FTransformComponent& Transform)
            {
                if (!Controller.Camera)
                {
                    return;
                }

                UpdateCamera(Controller, Transform, DeltaTime);
                UpdateMovement(Controller, Transform, DeltaTime);
            }
        );
    }

    int GetPriority() const override { return 10; }

private:
    void UpdateCamera(FThirdPersonControllerComponent& Controller,
                      const ECS::FTransformComponent& Transform,
                      float DeltaTime)
    {
        // Mouse input for camera orbit
        Math::Vec2 MousePos = Platform::GetMousePos();
        static float LastX = MousePos.x;
        static float LastY = MousePos.y;
        
        float DeltaX = MousePos.x - LastX;
        float DeltaY = MousePos.y - LastY;
        LastX = MousePos.x;
        LastY = MousePos.y;

        // Orbit camera around player
        Controller.Camera->Orbit(-DeltaX, -DeltaY, 0);
        
        // Update target position
        Controller.Camera->TargetPosition = Transform.Position + Math::Vec3{0, Controller.CameraHeight, 0};
        Controller.Camera->TargetDistance = Controller.CameraDistance;
    }

    void UpdateMovement(FThirdPersonControllerComponent& Controller,
                        ECS::FTransformComponent& Transform,
                        float DeltaTime)
    {
        // Get input
        Math::Vec3 InputDir{0, 0, 0};
        if (Platform::GetKeyDown(KeyCode::W)) InputDir.z += 1.0f;
        if (Platform::GetKeyDown(KeyCode::S)) InputDir.z -= 1.0f;
        if (Platform::GetKeyDown(KeyCode::A)) InputDir.x -= 1.0f;
        if (Platform::GetKeyDown(KeyCode::D)) InputDir.x += 1.0f;

        float InputLength = std::sqrt(InputDir.x * InputDir.x + InputDir.z * InputDir.z);
        
        if (InputLength > 0.1f)
        {
            InputDir.x /= InputLength;
            InputDir.z /= InputLength;

            // Get camera-relative movement direction
            Math::Vec3 CamForward = Controller.Camera->GetForward();
            Math::Vec3 CamRight = Controller.Camera->GetRight();
            CamForward.y = 0;
            CamForward = CamForward.Normalized();
            CamRight.y = 0;
            CamRight = CamRight.Normalized();

            Math::Vec3 MoveDir = CamForward * InputDir.z + CamRight * InputDir.x;
            MoveDir = MoveDir.Normalized();

            // Calculate target yaw from movement direction
            Controller.TargetYaw = std::atan2(MoveDir.x, MoveDir.z);

            // Smoothly rotate character
            float YawDiff = Controller.TargetYaw - Controller.CurrentYaw;
            while (YawDiff > Math::PI) YawDiff -= Math::TAU;
            while (YawDiff < -Math::PI) YawDiff += Math::TAU;
            
            Controller.CurrentYaw += YawDiff * Controller.TurnSpeed * DeltaTime;
            Transform.Rotation = Controller.CurrentYaw;

            // Move in facing direction
            Controller.bIsSprinting = Platform::GetKeyDown(KeyCode::LeftShift);
            float Speed = Controller.bIsSprinting ? Controller.SprintSpeed : Controller.MoveSpeed;
            
            Controller.Velocity.x = std::sin(Controller.CurrentYaw) * Speed;
            Controller.Velocity.z = std::cos(Controller.CurrentYaw) * Speed;
        }
        else
        {
            // Decelerate
            Controller.Velocity.x *= 0.9f;
            Controller.Velocity.z *= 0.9f;
        }

        // Gravity
        if (!Controller.bIsGrounded)
        {
            Controller.Velocity.y -= Controller.Gravity * DeltaTime;
        }

        // Jump
        if (Platform::GetKeyDown(KeyCode::Space) && Controller.bIsGrounded)
        {
            Controller.Velocity.y = Controller.JumpForce;
            Controller.bIsGrounded = false;
        }

        // Apply velocity
        Transform.Position.x += Controller.Velocity.x * DeltaTime;
        Transform.Position.y += Controller.Velocity.y * DeltaTime;
        Transform.Position.z += Controller.Velocity.z * DeltaTime;

        // Ground check
        if (Transform.Position.y <= 0.0f)
        {
            Transform.Position.y = 0.0f;
            Controller.Velocity.y = 0;
            Controller.bIsGrounded = true;
        }
    }
};

} // namespace Titan

#endif // TITAN_FPS_CONTROLLER_HPP

