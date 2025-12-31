/**
 * Titan 3D Camera System
 * 
 * Full-featured 3D camera with:
 * - Perspective/Orthographic projection
 * - FPS/TPS/Free camera modes
 * - Smooth movement and rotation
 * - View frustum for culling
 */

#ifndef TITAN_CAMERA_3D_HPP
#define TITAN_CAMERA_3D_HPP

#include "../Titan_Core.hpp"
#include <cmath>
#include <algorithm>

namespace Titan {

// ============================================================================
// Camera Enums
// ============================================================================

enum class ECameraMode
{
    Free,           // Free flying camera
    FirstPerson,    // FPS camera attached to entity
    ThirdPerson,    // TPS camera following entity
    Orbital         // Orbiting around target
};

enum class EProjectionType
{
    Perspective,
    Orthographic
};

// ============================================================================
// View Frustum for Culling
// ============================================================================

struct FPlane
{
    Math::Vec3 Normal;
    float Distance;

    FPlane() : Normal{0, 1, 0}, Distance(0) {}
    
    FPlane(const Math::Vec3& InNormal, float InDistance)
        : Normal(InNormal.Normalized()), Distance(InDistance) {}
    
    FPlane(const Math::Vec3& P1, const Math::Vec3& P2, const Math::Vec3& P3)
    {
        Math::Vec3 V1 = P2 - P1;
        Math::Vec3 V2 = P3 - P1;
        Normal = V1.Cross(V2).Normalized();
        Distance = -Normal.Dot(P1);
    }

    float DistanceToPoint(const Math::Vec3& Point) const
    {
        return Normal.Dot(Point) + Distance;
    }

    void Normalize()
    {
        float Length = Normal.Length();
        if (Length > 0.0001f)
        {
            Normal = Normal / Length;
            Distance /= Length;
        }
    }
};

struct FFrustum
{
    enum EPlaneIndex
    {
        Left = 0,
        Right = 1,
        Bottom = 2,
        Top = 3,
        Near = 4,
        Far = 5
    };

    FPlane Planes[6];

    void ExtractFromMatrix(const Math::Mat4& ViewProj)
    {
        // Left plane
        Planes[Left].Normal.x = ViewProj.m[0][3] + ViewProj.m[0][0];
        Planes[Left].Normal.y = ViewProj.m[1][3] + ViewProj.m[1][0];
        Planes[Left].Normal.z = ViewProj.m[2][3] + ViewProj.m[2][0];
        Planes[Left].Distance = ViewProj.m[3][3] + ViewProj.m[3][0];
        Planes[Left].Normalize();

        // Right plane
        Planes[Right].Normal.x = ViewProj.m[0][3] - ViewProj.m[0][0];
        Planes[Right].Normal.y = ViewProj.m[1][3] - ViewProj.m[1][0];
        Planes[Right].Normal.z = ViewProj.m[2][3] - ViewProj.m[2][0];
        Planes[Right].Distance = ViewProj.m[3][3] - ViewProj.m[3][0];
        Planes[Right].Normalize();

        // Bottom plane
        Planes[Bottom].Normal.x = ViewProj.m[0][3] + ViewProj.m[0][1];
        Planes[Bottom].Normal.y = ViewProj.m[1][3] + ViewProj.m[1][1];
        Planes[Bottom].Normal.z = ViewProj.m[2][3] + ViewProj.m[2][1];
        Planes[Bottom].Distance = ViewProj.m[3][3] + ViewProj.m[3][1];
        Planes[Bottom].Normalize();

        // Top plane
        Planes[Top].Normal.x = ViewProj.m[0][3] - ViewProj.m[0][1];
        Planes[Top].Normal.y = ViewProj.m[1][3] - ViewProj.m[1][1];
        Planes[Top].Normal.z = ViewProj.m[2][3] - ViewProj.m[2][1];
        Planes[Top].Distance = ViewProj.m[3][3] - ViewProj.m[3][1];
        Planes[Top].Normalize();

        // Near plane
        Planes[Near].Normal.x = ViewProj.m[0][3] + ViewProj.m[0][2];
        Planes[Near].Normal.y = ViewProj.m[1][3] + ViewProj.m[1][2];
        Planes[Near].Normal.z = ViewProj.m[2][3] + ViewProj.m[2][2];
        Planes[Near].Distance = ViewProj.m[3][3] + ViewProj.m[3][2];
        Planes[Near].Normalize();

        // Far plane
        Planes[Far].Normal.x = ViewProj.m[0][3] - ViewProj.m[0][2];
        Planes[Far].Normal.y = ViewProj.m[1][3] - ViewProj.m[1][2];
        Planes[Far].Normal.z = ViewProj.m[2][3] - ViewProj.m[2][2];
        Planes[Far].Distance = ViewProj.m[3][3] - ViewProj.m[3][2];
        Planes[Far].Normalize();
    }

    bool TestPoint(const Math::Vec3& Point) const
    {
        for (int i = 0; i < 6; i++)
        {
            if (Planes[i].DistanceToPoint(Point) < 0)
            {
                return false;
            }
        }
        return true;
    }

    bool TestSphere(const Math::Vec3& Center, float Radius) const
    {
        for (int i = 0; i < 6; i++)
        {
            if (Planes[i].DistanceToPoint(Center) < -Radius)
            {
                return false;
            }
        }
        return true;
    }

    bool TestAABB(const Math::Vec3& Min, const Math::Vec3& Max) const
    {
        for (int i = 0; i < 6; i++)
        {
            Math::Vec3 Positive = Min;
            if (Planes[i].Normal.x >= 0) Positive.x = Max.x;
            if (Planes[i].Normal.y >= 0) Positive.y = Max.y;
            if (Planes[i].Normal.z >= 0) Positive.z = Max.z;

            if (Planes[i].DistanceToPoint(Positive) < 0)
            {
                return false;
            }
        }
        return true;
    }
};

// ============================================================================
// 3D Camera Class
// ============================================================================

class FCamera3D
{
public:
    // Transform
    Math::Vec3 Position{0, 0, 5};
    Math::Vec3 Rotation{0, 0, 0};  // Pitch, Yaw, Roll in radians
    
    // Projection settings
    EProjectionType ProjectionType = EProjectionType::Perspective;
    float FOV = 70.0f;
    float NearPlane = 0.1f;
    float FarPlane = 1000.0f;
    float AspectRatio = 16.0f / 9.0f;
    float OrthoSize = 10.0f;

    // Camera mode
    ECameraMode Mode = ECameraMode::Free;
    
    // Third person settings
    Math::Vec3 TargetPosition{0, 0, 0};
    float TargetDistance = 5.0f;
    Math::Vec3 TargetOffset{0, 1.5f, 0};

    // Sensitivity
    float MouseSensitivity = 0.002f;
    float MoveSpeed = 10.0f;
    float SprintMultiplier = 2.0f;

    // Constraints
    float MinPitch = -Math::PI * 0.49f;
    float MaxPitch = Math::PI * 0.49f;

    // Smoothing
    bool bEnableSmoothing = true;
    float SmoothingSpeed = 15.0f;

    // Cached matrices
    Math::Mat4 ViewMatrix;
    Math::Mat4 ProjectionMatrix;
    Math::Mat4 ViewProjectionMatrix;
    FFrustum Frustum;

private:
    Math::Vec3 SmoothedPosition{0, 0, 5};
    Math::Vec3 SmoothedRotation{0, 0, 0};
    Math::Vec3 Forward{0, 0, -1};
    Math::Vec3 Right{1, 0, 0};
    Math::Vec3 Up{0, 1, 0};

public:
    FCamera3D() = default;

    /**
     * Update camera matrices and frustum
     */
    void Update(float DeltaTime)
    {
        // Apply smoothing
        if (bEnableSmoothing)
        {
            float T = 1.0f - std::exp(-SmoothingSpeed * DeltaTime);
            SmoothedPosition = Lerp(SmoothedPosition, Position, T);
            SmoothedRotation = Lerp(SmoothedRotation, Rotation, T);
        }
        else
        {
            SmoothedPosition = Position;
            SmoothedRotation = Rotation;
        }

        // Update direction vectors
        UpdateDirections();

        // Build matrices
        BuildViewMatrix();
        BuildProjectionMatrix();
        
        ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
        Frustum.ExtractFromMatrix(ViewProjectionMatrix);
    }

    /**
     * Rotate camera by mouse delta
     */
    void Rotate(float DeltaX, float DeltaY)
    {
        Rotation.y -= DeltaX * MouseSensitivity; // Yaw
        Rotation.x -= DeltaY * MouseSensitivity; // Pitch

        // Clamp pitch
        Rotation.x = std::clamp(Rotation.x, MinPitch, MaxPitch);

        // Wrap yaw
        while (Rotation.y > Math::PI) Rotation.y -= Math::TAU;
        while (Rotation.y < -Math::PI) Rotation.y += Math::TAU;
    }

    /**
     * Move camera in local space
     */
    void Move(const Math::Vec3& LocalDirection, float DeltaTime, bool bSprint = false)
    {
        float Speed = MoveSpeed * (bSprint ? SprintMultiplier : 1.0f);
        
        Math::Vec3 Movement{0, 0, 0};
        Movement = Movement + Right * LocalDirection.x;
        Movement = Movement + Up * LocalDirection.y;
        Movement = Movement + Forward * LocalDirection.z;

        Position = Position + Movement * Speed * DeltaTime;
    }

    /**
     * Move camera to follow target (for TPS mode)
     */
    void FollowTarget(const Math::Vec3& InTargetPosition, float DeltaTime)
    {
        TargetPosition = InTargetPosition + TargetOffset;
        
        // Calculate camera position behind target
        Math::Vec3 DesiredPosition = TargetPosition - Forward * TargetDistance;
        
        if (bEnableSmoothing)
        {
            float T = 1.0f - std::exp(-SmoothingSpeed * DeltaTime);
            Position = Lerp(Position, DesiredPosition, T);
        }
        else
        {
            Position = DesiredPosition;
        }
    }

    /**
     * Orbit around target point
     */
    void Orbit(float DeltaYaw, float DeltaPitch, float DeltaDistance)
    {
        Rotation.y += DeltaYaw * MouseSensitivity;
        Rotation.x += DeltaPitch * MouseSensitivity;
        Rotation.x = std::clamp(Rotation.x, MinPitch, MaxPitch);

        TargetDistance = std::max(0.5f, TargetDistance + DeltaDistance);

        // Calculate new position
        float CosP = std::cos(Rotation.x);
        float SinP = std::sin(Rotation.x);
        float CosY = std::cos(Rotation.y);
        float SinY = std::sin(Rotation.y);

        Math::Vec3 Offset{
            SinY * CosP * TargetDistance,
            SinP * TargetDistance,
            CosY * CosP * TargetDistance
        };

        Position = TargetPosition + Offset;
    }

    /**
     * Set target to look at
     */
    void LookAt(const Math::Vec3& Target)
    {
        Math::Vec3 Dir = (Target - Position).Normalized();
        
        Rotation.x = std::asin(-Dir.y);
        Rotation.y = std::atan2(Dir.x, Dir.z);
    }

    /**
     * Get forward direction
     */
    Math::Vec3 GetForward() const { return Forward; }
    
    /**
     * Get right direction
     */
    Math::Vec3 GetRight() const { return Right; }
    
    /**
     * Get up direction
     */
    Math::Vec3 GetUp() const { return Up; }

    /**
     * Get world position (smoothed)
     */
    Math::Vec3 GetPosition() const 
    { 
        return bEnableSmoothing ? SmoothedPosition : Position; 
    }

    /**
     * Screen to world ray
     */
    void ScreenToWorldRay(float ScreenX, float ScreenY, float ScreenWidth, float ScreenHeight,
                          Math::Vec3& OutOrigin, Math::Vec3& OutDirection) const
    {
        // Convert to NDC [-1, 1]
        float NdcX = (2.0f * ScreenX / ScreenWidth) - 1.0f;
        float NdcY = 1.0f - (2.0f * ScreenY / ScreenHeight);

        // Inverse projection
        float TanFov = std::tan(FOV * 0.5f * Math::DEG_TO_RAD);
        
        Math::Vec3 RayDir{
            NdcX * AspectRatio * TanFov,
            NdcY * TanFov,
            -1.0f
        };

        // Transform to world space
        OutOrigin = GetPosition();
        OutDirection = TransformDirection(RayDir.Normalized());
    }

    /**
     * World to screen position
     */
    bool WorldToScreen(const Math::Vec3& WorldPos, float ScreenWidth, float ScreenHeight,
                       float& OutX, float& OutY) const
    {
        // Transform to clip space
        Math::Vec4 ClipPos = TransformPoint4(WorldPos, ViewProjectionMatrix);
        
        if (ClipPos.w <= 0.0f)
        {
            return false; // Behind camera
        }

        // Perspective divide
        float NdcX = ClipPos.x / ClipPos.w;
        float NdcY = ClipPos.y / ClipPos.w;
        float NdcZ = ClipPos.z / ClipPos.w;

        if (NdcZ < -1.0f || NdcZ > 1.0f)
        {
            return false; // Outside frustum
        }

        // Convert to screen coordinates
        OutX = (NdcX + 1.0f) * 0.5f * ScreenWidth;
        OutY = (1.0f - NdcY) * 0.5f * ScreenHeight;

        return true;
    }

    /**
     * Test if point is in view frustum
     */
    bool IsPointVisible(const Math::Vec3& Point) const
    {
        return Frustum.TestPoint(Point);
    }

    /**
     * Test if sphere is in view frustum
     */
    bool IsSphereVisible(const Math::Vec3& Center, float Radius) const
    {
        return Frustum.TestSphere(Center, Radius);
    }

    /**
     * Test if AABB is in view frustum
     */
    bool IsAABBVisible(const Math::Vec3& Min, const Math::Vec3& Max) const
    {
        return Frustum.TestAABB(Min, Max);
    }

private:
    void UpdateDirections()
    {
        float CosP = std::cos(SmoothedRotation.x);
        float SinP = std::sin(SmoothedRotation.x);
        float CosY = std::cos(SmoothedRotation.y);
        float SinY = std::sin(SmoothedRotation.y);
        float CosR = std::cos(SmoothedRotation.z);
        float SinR = std::sin(SmoothedRotation.z);

        Forward = Math::Vec3{
            SinY * CosP,
            -SinP,
            -CosY * CosP
        }.Normalized();

        Right = Math::Vec3{
            CosY * CosR + SinY * SinP * SinR,
            CosP * SinR,
            -SinY * CosR + CosY * SinP * SinR
        }.Normalized();

        Up = Right.Cross(Forward).Normalized();
    }

    void BuildViewMatrix()
    {
        Math::Vec3 Pos = bEnableSmoothing ? SmoothedPosition : Position;
        Math::Vec3 Target = Pos + Forward;

        // LookAt matrix
        Math::Vec3 F = Forward;
        Math::Vec3 R = Right;
        Math::Vec3 U = Up;

        ViewMatrix = Math::Mat4();
        ViewMatrix.m[0][0] = R.x;
        ViewMatrix.m[1][0] = R.y;
        ViewMatrix.m[2][0] = R.z;
        ViewMatrix.m[3][0] = -R.Dot(Pos);

        ViewMatrix.m[0][1] = U.x;
        ViewMatrix.m[1][1] = U.y;
        ViewMatrix.m[2][1] = U.z;
        ViewMatrix.m[3][1] = -U.Dot(Pos);

        ViewMatrix.m[0][2] = -F.x;
        ViewMatrix.m[1][2] = -F.y;
        ViewMatrix.m[2][2] = -F.z;
        ViewMatrix.m[3][2] = F.Dot(Pos);

        ViewMatrix.m[0][3] = 0.0f;
        ViewMatrix.m[1][3] = 0.0f;
        ViewMatrix.m[2][3] = 0.0f;
        ViewMatrix.m[3][3] = 1.0f;
    }

    void BuildProjectionMatrix()
    {
        if (ProjectionType == EProjectionType::Perspective)
        {
            ProjectionMatrix = Math::Mat4::Perspective(FOV, AspectRatio, NearPlane, FarPlane);
        }
        else
        {
            float HalfHeight = OrthoSize * 0.5f;
            float HalfWidth = HalfHeight * AspectRatio;
            ProjectionMatrix = Math::Mat4::Ortho(-HalfWidth, HalfWidth, -HalfHeight, HalfHeight, NearPlane, FarPlane);
        }
    }

    Math::Vec3 Lerp(const Math::Vec3& A, const Math::Vec3& B, float T) const
    {
        return Math::Vec3{
            A.x + (B.x - A.x) * T,
            A.y + (B.y - A.y) * T,
            A.z + (B.z - A.z) * T
        };
    }

    Math::Vec3 TransformDirection(const Math::Vec3& Dir) const
    {
        // Rotate direction by camera orientation
        return Math::Vec3{
            Right.x * Dir.x + Up.x * Dir.y - Forward.x * Dir.z,
            Right.y * Dir.x + Up.y * Dir.y - Forward.y * Dir.z,
            Right.z * Dir.x + Up.z * Dir.y - Forward.z * Dir.z
        };
    }

    Math::Vec4 TransformPoint4(const Math::Vec3& P, const Math::Mat4& M) const
    {
        return Math::Vec4{
            M.m[0][0] * P.x + M.m[1][0] * P.y + M.m[2][0] * P.z + M.m[3][0],
            M.m[0][1] * P.x + M.m[1][1] * P.y + M.m[2][1] * P.z + M.m[3][1],
            M.m[0][2] * P.x + M.m[1][2] * P.y + M.m[2][2] * P.z + M.m[3][2],
            M.m[0][3] * P.x + M.m[1][3] * P.y + M.m[2][3] * P.z + M.m[3][3]
        };
    }
};

// ============================================================================
// FPS Controller Component
// ============================================================================

struct FFPSControllerComponent
{
    // Movement
    float MoveSpeed = 7.0f;
    float SprintSpeed = 12.0f;
    float JumpForce = 8.0f;
    float Gravity = 20.0f;
    
    // Mouse look
    float MouseSensitivity = 0.15f;
    bool bInvertY = false;
    
    // State
    Math::Vec3 Velocity{0, 0, 0};
    bool bIsGrounded = false;
    bool bIsSprinting = false;
    bool bIsCrouching = false;
    
    // Heights
    float StandingHeight = 1.8f;
    float CrouchingHeight = 1.0f;
    float EyeHeight = 0.9f; // Relative to center
    
    // Physics
    float Mass = 80.0f;
    float Friction = 8.0f;
    float AirControl = 0.3f;
    
    // Camera reference
    FCamera3D* Camera = nullptr;
    
    // Footstep
    float FootstepTimer = 0.0f;
    float FootstepInterval = 0.5f;
};

} // namespace Titan

#endif // TITAN_CAMERA_3D_HPP

