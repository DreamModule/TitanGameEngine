/**
 * Titan Skeletal Animation System
 * 
 * Full-featured animation system with:
 * - Bone hierarchy
 * - Animation clips with keyframes
 * - Animation blending (crossfade, additive)
 * - Animation layers
 * - IK ready
 */

#ifndef TITAN_SKELETAL_ANIMATION_HPP
#define TITAN_SKELETAL_ANIMATION_HPP

#include "../Titan_Core.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace Titan {

// ============================================================================
// Bone/Joint
// ============================================================================

struct FBone
{
    std::string Name;
    int ParentIndex = -1;
    
    // Bind pose (inverse bind matrix)
    Math::Mat4 InverseBindPose;
    
    // Local transform relative to parent
    Math::Vec3 LocalPosition{0, 0, 0};
    Math::Quaternion LocalRotation = Math::Quaternion::Identity();
    Math::Vec3 LocalScale{1, 1, 1};
    
    // Final world transform (computed each frame)
    Math::Mat4 WorldTransform;
    
    // Skinning matrix (WorldTransform * InverseBindPose)
    Math::Mat4 SkinningMatrix;
};

// ============================================================================
// Skeleton
// ============================================================================

class FSkeleton
{
public:
    std::vector<FBone> Bones;
    std::unordered_map<std::string, int> BoneNameToIndex;
    std::vector<int> RootBones; // Bones with no parent

    void AddBone(const FBone& Bone)
    {
        int Index = static_cast<int>(Bones.size());
        BoneNameToIndex[Bone.Name] = Index;
        Bones.push_back(Bone);
        
        if (Bone.ParentIndex == -1)
        {
            RootBones.push_back(Index);
        }
    }

    int FindBone(const std::string& Name) const
    {
        auto It = BoneNameToIndex.find(Name);
        if (It != BoneNameToIndex.end())
        {
            return It->second;
        }
        return -1;
    }

    void CalculateWorldTransforms()
    {
        for (int RootIdx : RootBones)
        {
            CalculateBoneTransform(RootIdx, Math::Mat4::Identity());
        }
    }

    const Math::Mat4* GetSkinningMatrices() const
    {
        return Bones.empty() ? nullptr : &Bones[0].SkinningMatrix;
    }

    size_t GetBoneCount() const { return Bones.size(); }

private:
    void CalculateBoneTransform(int BoneIdx, const Math::Mat4& ParentTransform)
    {
        FBone& Bone = Bones[BoneIdx];
        
        // Build local transform
        Math::Mat4 LocalTransform = Math::Mat4::Translate(Bone.LocalPosition) *
                                     QuaternionToMatrix(Bone.LocalRotation) *
                                     Math::Mat4::Scale(Bone.LocalScale);
        
        // World transform
        Bone.WorldTransform = ParentTransform * LocalTransform;
        
        // Skinning matrix
        Bone.SkinningMatrix = Bone.WorldTransform * Bone.InverseBindPose;
        
        // Process children
        for (size_t i = 0; i < Bones.size(); i++)
        {
            if (Bones[i].ParentIndex == BoneIdx)
            {
                CalculateBoneTransform(static_cast<int>(i), Bone.WorldTransform);
            }
        }
    }

    Math::Mat4 QuaternionToMatrix(const Math::Quaternion& Q) const
    {
        Math::Mat4 M;
        
        float XX = Q.x * Q.x;
        float YY = Q.y * Q.y;
        float ZZ = Q.z * Q.z;
        float XY = Q.x * Q.y;
        float XZ = Q.x * Q.z;
        float YZ = Q.y * Q.z;
        float WX = Q.w * Q.x;
        float WY = Q.w * Q.y;
        float WZ = Q.w * Q.z;
        
        M.m[0][0] = 1.0f - 2.0f * (YY + ZZ);
        M.m[0][1] = 2.0f * (XY + WZ);
        M.m[0][2] = 2.0f * (XZ - WY);
        M.m[0][3] = 0.0f;
        
        M.m[1][0] = 2.0f * (XY - WZ);
        M.m[1][1] = 1.0f - 2.0f * (XX + ZZ);
        M.m[1][2] = 2.0f * (YZ + WX);
        M.m[1][3] = 0.0f;
        
        M.m[2][0] = 2.0f * (XZ + WY);
        M.m[2][1] = 2.0f * (YZ - WX);
        M.m[2][2] = 1.0f - 2.0f * (XX + YY);
        M.m[2][3] = 0.0f;
        
        M.m[3][0] = 0.0f;
        M.m[3][1] = 0.0f;
        M.m[3][2] = 0.0f;
        M.m[3][3] = 1.0f;
        
        return M;
    }
};

// ============================================================================
// Animation Keyframe
// ============================================================================

template<typename T>
struct FKeyframe
{
    float Time;
    T Value;
};

// ============================================================================
// Animation Channel (one property of one bone)
// ============================================================================

template<typename T>
class FAnimationChannel
{
public:
    std::vector<FKeyframe<T>> Keyframes;

    T Sample(float Time) const
    {
        if (Keyframes.empty())
        {
            return T{};
        }

        if (Time <= Keyframes.front().Time)
        {
            return Keyframes.front().Value;
        }

        if (Time >= Keyframes.back().Time)
        {
            return Keyframes.back().Value;
        }

        // Find keyframes to interpolate between
        for (size_t i = 0; i < Keyframes.size() - 1; i++)
        {
            if (Time >= Keyframes[i].Time && Time <= Keyframes[i + 1].Time)
            {
                float T = (Time - Keyframes[i].Time) / 
                          (Keyframes[i + 1].Time - Keyframes[i].Time);
                return Lerp(Keyframes[i].Value, Keyframes[i + 1].Value, T);
            }
        }

        return Keyframes.back().Value;
    }

private:
    // Linear interpolation for Vec3
    Math::Vec3 Lerp(const Math::Vec3& A, const Math::Vec3& B, float T) const
    {
        return Math::Vec3{
            A.x + (B.x - A.x) * T,
            A.y + (B.y - A.y) * T,
            A.z + (B.z - A.z) * T
        };
    }

    // Spherical interpolation for Quaternion
    Math::Quaternion Lerp(const Math::Quaternion& A, const Math::Quaternion& B, float T) const
    {
        return Slerp(A, B, T);
    }

    Math::Quaternion Slerp(const Math::Quaternion& A, const Math::Quaternion& B, float T) const
    {
        Math::Quaternion Result;
        
        float Dot = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
        
        Math::Quaternion B2 = B;
        if (Dot < 0.0f)
        {
            Dot = -Dot;
            B2.x = -B2.x;
            B2.y = -B2.y;
            B2.z = -B2.z;
            B2.w = -B2.w;
        }

        if (Dot > 0.9995f)
        {
            // Linear interpolation for very close quaternions
            Result.x = A.x + (B2.x - A.x) * T;
            Result.y = A.y + (B2.y - A.y) * T;
            Result.z = A.z + (B2.z - A.z) * T;
            Result.w = A.w + (B2.w - A.w) * T;
            
            // Normalize
            float Len = std::sqrt(Result.x * Result.x + Result.y * Result.y + 
                                   Result.z * Result.z + Result.w * Result.w);
            Result.x /= Len;
            Result.y /= Len;
            Result.z /= Len;
            Result.w /= Len;
        }
        else
        {
            float Theta0 = std::acos(Dot);
            float Theta = Theta0 * T;
            float SinTheta = std::sin(Theta);
            float SinTheta0 = std::sin(Theta0);
            
            float S0 = std::cos(Theta) - Dot * SinTheta / SinTheta0;
            float S1 = SinTheta / SinTheta0;
            
            Result.x = A.x * S0 + B2.x * S1;
            Result.y = A.y * S0 + B2.y * S1;
            Result.z = A.z * S0 + B2.z * S1;
            Result.w = A.w * S0 + B2.w * S1;
        }

        return Result;
    }
};

// ============================================================================
// Animation Clip
// ============================================================================

struct FBoneAnimation
{
    int BoneIndex = -1;
    FAnimationChannel<Math::Vec3> PositionChannel;
    FAnimationChannel<Math::Quaternion> RotationChannel;
    FAnimationChannel<Math::Vec3> ScaleChannel;
};

class FAnimationClip
{
public:
    std::string Name;
    float Duration = 0.0f;
    bool bLooping = true;
    float TicksPerSecond = 30.0f;
    
    std::vector<FBoneAnimation> BoneAnimations;
    std::unordered_map<int, size_t> BoneIndexToAnimation;

    void AddBoneAnimation(const FBoneAnimation& Anim)
    {
        BoneIndexToAnimation[Anim.BoneIndex] = BoneAnimations.size();
        BoneAnimations.push_back(Anim);
    }

    bool HasBone(int BoneIndex) const
    {
        return BoneIndexToAnimation.find(BoneIndex) != BoneIndexToAnimation.end();
    }

    const FBoneAnimation* GetBoneAnimation(int BoneIndex) const
    {
        auto It = BoneIndexToAnimation.find(BoneIndex);
        if (It != BoneIndexToAnimation.end())
        {
            return &BoneAnimations[It->second];
        }
        return nullptr;
    }

    void Sample(float Time, FSkeleton& Skeleton) const
    {
        for (const auto& BoneAnim : BoneAnimations)
        {
            if (BoneAnim.BoneIndex >= 0 && 
                BoneAnim.BoneIndex < static_cast<int>(Skeleton.Bones.size()))
            {
                FBone& Bone = Skeleton.Bones[BoneAnim.BoneIndex];
                
                if (!BoneAnim.PositionChannel.Keyframes.empty())
                {
                    Bone.LocalPosition = BoneAnim.PositionChannel.Sample(Time);
                }
                
                if (!BoneAnim.RotationChannel.Keyframes.empty())
                {
                    Bone.LocalRotation = BoneAnim.RotationChannel.Sample(Time);
                }
                
                if (!BoneAnim.ScaleChannel.Keyframes.empty())
                {
                    Bone.LocalScale = BoneAnim.ScaleChannel.Sample(Time);
                }
            }
        }
    }
};

// ============================================================================
// Animation State
// ============================================================================

struct FAnimationState
{
    FAnimationClip* Clip = nullptr;
    float Time = 0.0f;
    float Speed = 1.0f;
    float Weight = 1.0f;
    bool bPlaying = false;
    bool bFinished = false;

    void Update(float DeltaTime)
    {
        if (!bPlaying || !Clip)
        {
            return;
        }

        Time += DeltaTime * Speed;

        if (Clip->bLooping)
        {
            while (Time >= Clip->Duration)
            {
                Time -= Clip->Duration;
            }
            while (Time < 0)
            {
                Time += Clip->Duration;
            }
        }
        else
        {
            if (Time >= Clip->Duration)
            {
                Time = Clip->Duration;
                bFinished = true;
                bPlaying = false;
            }
            else if (Time < 0)
            {
                Time = 0;
                bFinished = true;
                bPlaying = false;
            }
        }
    }

    void Play()
    {
        bPlaying = true;
        bFinished = false;
    }

    void Stop()
    {
        bPlaying = false;
    }

    void Reset()
    {
        Time = 0.0f;
        bFinished = false;
    }
};

// ============================================================================
// Animation Blend Node
// ============================================================================

enum class EBlendMode
{
    Override,
    Additive,
    Multiply
};

struct FBlendNode
{
    FAnimationState State;
    EBlendMode BlendMode = EBlendMode::Override;
    int LayerIndex = 0;
    
    // Mask (which bones this affects, empty = all)
    std::vector<int> BoneMask;
};

// ============================================================================
// Animation Controller
// ============================================================================

class FAnimationController
{
public:
    FSkeleton* Skeleton = nullptr;
    std::vector<FBlendNode> Layers;
    
    // Crossfade
    bool bCrossfading = false;
    float CrossfadeTime = 0.0f;
    float CrossfadeDuration = 0.3f;
    int CrossfadeFromLayer = -1;
    int CrossfadeToLayer = -1;

    void Update(float DeltaTime)
    {
        if (!Skeleton)
        {
            return;
        }

        // Update all animation states
        for (auto& Layer : Layers)
        {
            Layer.State.Update(DeltaTime);
        }

        // Handle crossfade
        if (bCrossfading)
        {
            CrossfadeTime += DeltaTime;
            float T = CrossfadeTime / CrossfadeDuration;
            
            if (T >= 1.0f)
            {
                T = 1.0f;
                bCrossfading = false;
                
                if (CrossfadeFromLayer >= 0 && CrossfadeFromLayer < static_cast<int>(Layers.size()))
                {
                    Layers[CrossfadeFromLayer].State.Weight = 0.0f;
                }
            }
            
            if (CrossfadeFromLayer >= 0 && CrossfadeFromLayer < static_cast<int>(Layers.size()))
            {
                Layers[CrossfadeFromLayer].State.Weight = 1.0f - T;
            }
            if (CrossfadeToLayer >= 0 && CrossfadeToLayer < static_cast<int>(Layers.size()))
            {
                Layers[CrossfadeToLayer].State.Weight = T;
            }
        }

        // Reset skeleton to bind pose
        ResetSkeletonToBind();

        // Apply animations in layer order
        std::sort(Layers.begin(), Layers.end(), 
            [](const FBlendNode& A, const FBlendNode& B) 
            { 
                return A.LayerIndex < B.LayerIndex; 
            });

        for (const auto& Layer : Layers)
        {
            if (Layer.State.Clip && Layer.State.Weight > 0.001f)
            {
                ApplyAnimation(Layer);
            }
        }

        // Calculate final transforms
        Skeleton->CalculateWorldTransforms();
    }

    int AddLayer(FAnimationClip* Clip)
    {
        FBlendNode Node;
        Node.State.Clip = Clip;
        Node.State.Time = 0.0f;
        Node.State.Weight = 1.0f;
        Node.LayerIndex = static_cast<int>(Layers.size());
        Layers.push_back(Node);
        return static_cast<int>(Layers.size()) - 1;
    }

    void Play(int LayerIndex)
    {
        if (LayerIndex >= 0 && LayerIndex < static_cast<int>(Layers.size()))
        {
            Layers[LayerIndex].State.Play();
        }
    }

    void PlayClip(FAnimationClip* Clip)
    {
        if (Layers.empty())
        {
            AddLayer(Clip);
        }
        else
        {
            Layers[0].State.Clip = Clip;
            Layers[0].State.Reset();
        }
        Layers[0].State.Play();
    }

    void CrossfadeTo(int LayerIndex, float Duration = 0.3f)
    {
        if (LayerIndex < 0 || LayerIndex >= static_cast<int>(Layers.size()))
        {
            return;
        }

        // Find currently playing layer
        int CurrentLayer = -1;
        for (size_t i = 0; i < Layers.size(); i++)
        {
            if (Layers[i].State.bPlaying && Layers[i].State.Weight > 0.5f)
            {
                CurrentLayer = static_cast<int>(i);
                break;
            }
        }

        if (CurrentLayer == LayerIndex)
        {
            return;
        }

        bCrossfading = true;
        CrossfadeTime = 0.0f;
        CrossfadeDuration = Duration;
        CrossfadeFromLayer = CurrentLayer;
        CrossfadeToLayer = LayerIndex;

        Layers[LayerIndex].State.Play();
        Layers[LayerIndex].State.Reset();
    }

    void CrossfadeToClip(FAnimationClip* Clip, float Duration = 0.3f)
    {
        // Find or create layer for this clip
        int TargetLayer = -1;
        for (size_t i = 0; i < Layers.size(); i++)
        {
            if (Layers[i].State.Clip == Clip)
            {
                TargetLayer = static_cast<int>(i);
                break;
            }
        }

        if (TargetLayer == -1)
        {
            TargetLayer = AddLayer(Clip);
            Layers[TargetLayer].State.Weight = 0.0f;
        }

        CrossfadeTo(TargetLayer, Duration);
    }

    void SetSpeed(int LayerIndex, float Speed)
    {
        if (LayerIndex >= 0 && LayerIndex < static_cast<int>(Layers.size()))
        {
            Layers[LayerIndex].State.Speed = Speed;
        }
    }

    float GetTime(int LayerIndex) const
    {
        if (LayerIndex >= 0 && LayerIndex < static_cast<int>(Layers.size()))
        {
            return Layers[LayerIndex].State.Time;
        }
        return 0.0f;
    }

    float GetNormalizedTime(int LayerIndex) const
    {
        if (LayerIndex >= 0 && LayerIndex < static_cast<int>(Layers.size()))
        {
            const auto& State = Layers[LayerIndex].State;
            if (State.Clip && State.Clip->Duration > 0)
            {
                return State.Time / State.Clip->Duration;
            }
        }
        return 0.0f;
    }

private:
    void ResetSkeletonToBind()
    {
        for (auto& Bone : Skeleton->Bones)
        {
            Bone.LocalPosition = Math::Vec3{0, 0, 0};
            Bone.LocalRotation = Math::Quaternion::Identity();
            Bone.LocalScale = Math::Vec3{1, 1, 1};
        }
    }

    void ApplyAnimation(const FBlendNode& Node)
    {
        const auto& State = Node.State;
        
        // Sample animation at current time
        for (const auto& BoneAnim : State.Clip->BoneAnimations)
        {
            int BoneIdx = BoneAnim.BoneIndex;
            
            if (BoneIdx < 0 || BoneIdx >= static_cast<int>(Skeleton->Bones.size()))
            {
                continue;
            }

            // Check bone mask
            if (!Node.BoneMask.empty())
            {
                bool Found = false;
                for (int MaskedBone : Node.BoneMask)
                {
                    if (MaskedBone == BoneIdx)
                    {
                        Found = true;
                        break;
                    }
                }
                if (!Found) continue;
            }

            FBone& Bone = Skeleton->Bones[BoneIdx];
            
            // Sample channels
            Math::Vec3 Position = BoneAnim.PositionChannel.Keyframes.empty() ? 
                Bone.LocalPosition : BoneAnim.PositionChannel.Sample(State.Time);
            
            Math::Quaternion Rotation = BoneAnim.RotationChannel.Keyframes.empty() ?
                Bone.LocalRotation : BoneAnim.RotationChannel.Sample(State.Time);
            
            Math::Vec3 Scale = BoneAnim.ScaleChannel.Keyframes.empty() ?
                Bone.LocalScale : BoneAnim.ScaleChannel.Sample(State.Time);

            // Apply based on blend mode
            float W = State.Weight;

            switch (Node.BlendMode)
            {
                case EBlendMode::Override:
                    Bone.LocalPosition = Lerp(Bone.LocalPosition, Position, W);
                    Bone.LocalRotation = Slerp(Bone.LocalRotation, Rotation, W);
                    Bone.LocalScale = Lerp(Bone.LocalScale, Scale, W);
                    break;

                case EBlendMode::Additive:
                    Bone.LocalPosition = Bone.LocalPosition + Position * W;
                    // For additive rotation, we multiply quaternions
                    Bone.LocalRotation = Bone.LocalRotation * Slerp(
                        Math::Quaternion::Identity(), Rotation, W);
                    Bone.LocalScale = Bone.LocalScale + (Scale - Math::Vec3{1,1,1}) * W;
                    break;

                case EBlendMode::Multiply:
                    Bone.LocalPosition.x *= 1.0f + (Position.x - 1.0f) * W;
                    Bone.LocalPosition.y *= 1.0f + (Position.y - 1.0f) * W;
                    Bone.LocalPosition.z *= 1.0f + (Position.z - 1.0f) * W;
                    Bone.LocalScale.x *= 1.0f + (Scale.x - 1.0f) * W;
                    Bone.LocalScale.y *= 1.0f + (Scale.y - 1.0f) * W;
                    Bone.LocalScale.z *= 1.0f + (Scale.z - 1.0f) * W;
                    break;
            }
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

    Math::Quaternion Slerp(const Math::Quaternion& A, const Math::Quaternion& B, float T) const
    {
        Math::Quaternion Result;
        
        float Dot = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
        
        Math::Quaternion B2 = B;
        if (Dot < 0.0f)
        {
            Dot = -Dot;
            B2.x = -B2.x;
            B2.y = -B2.y;
            B2.z = -B2.z;
            B2.w = -B2.w;
        }

        if (Dot > 0.9995f)
        {
            Result.x = A.x + (B2.x - A.x) * T;
            Result.y = A.y + (B2.y - A.y) * T;
            Result.z = A.z + (B2.z - A.z) * T;
            Result.w = A.w + (B2.w - A.w) * T;
            
            float Len = std::sqrt(Result.x * Result.x + Result.y * Result.y + 
                                   Result.z * Result.z + Result.w * Result.w);
            Result.x /= Len;
            Result.y /= Len;
            Result.z /= Len;
            Result.w /= Len;
        }
        else
        {
            float Theta0 = std::acos(Dot);
            float Theta = Theta0 * T;
            float SinTheta = std::sin(Theta);
            float SinTheta0 = std::sin(Theta0);
            
            float S0 = std::cos(Theta) - Dot * SinTheta / SinTheta0;
            float S1 = SinTheta / SinTheta0;
            
            Result.x = A.x * S0 + B2.x * S1;
            Result.y = A.y * S0 + B2.y * S1;
            Result.z = A.z * S0 + B2.z * S1;
            Result.w = A.w * S0 + B2.w * S1;
        }

        return Result;
    }
};

// ============================================================================
// Animation Component for ECS
// ============================================================================

struct FSkeletalMeshComponent
{
    FSkeleton Skeleton;
    FAnimationController AnimController;
    
    // Mesh reference
    FMesh3D* Mesh = nullptr;
    
    // Animation clips
    std::vector<FAnimationClip> AnimationClips;
    
    void Initialize()
    {
        AnimController.Skeleton = &Skeleton;
    }
    
    void Update(float DeltaTime)
    {
        AnimController.Update(DeltaTime);
    }
    
    void PlayAnimation(const std::string& Name, bool bCrossfade = true, float CrossfadeTime = 0.3f)
    {
        for (auto& Clip : AnimationClips)
        {
            if (Clip.Name == Name)
            {
                if (bCrossfade)
                {
                    AnimController.CrossfadeToClip(&Clip, CrossfadeTime);
                }
                else
                {
                    AnimController.PlayClip(&Clip);
                }
                return;
            }
        }
    }
    
    const Math::Mat4* GetBoneMatrices() const
    {
        return Skeleton.GetSkinningMatrices();
    }
    
    size_t GetBoneCount() const
    {
        return Skeleton.GetBoneCount();
    }
};

} // namespace Titan

#endif // TITAN_SKELETAL_ANIMATION_HPP


