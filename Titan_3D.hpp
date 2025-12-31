/**
 * Titan 3D Engine Module
 * 
 * Main header that includes all 3D functionality:
 * - Camera and Controllers
 * - 3D Rendering with PBR
 * - Model Loading (GLTF)
 * - Skeletal Animation
 * - 3D Physics
 * 
 * Usage:
 *   #include "Titan_3D.hpp"
 *   
 *   // Create camera
 *   Titan::FCamera3D Camera;
 *   Camera.Position = {0, 2, 5};
 *   Camera.FOV = 70.0f;
 *   
 *   // Load model
 *   Titan::FModel3D Model;
 *   Titan::FGLTFLoader::Load("assets/player.glb", Model);
 *   
 *   // Setup physics
 *   Titan::Physics::FPhysicsWorld PhysWorld;
 *   PhysWorld.Initialize(&World);
 */

#ifndef TITAN_3D_HPP
#define TITAN_3D_HPP

// Core 3D components
#include "src/3D/Camera3D.hpp"
#include "src/3D/FPSController.hpp"
#include "src/3D/Renderer3D.hpp"
#include "src/3D/GLTFLoader.hpp"
#include "src/3D/SkeletalAnimation.hpp"
#include "src/3D/Physics3D.hpp"

namespace Titan {

// ============================================================================
// 3D Engine Context
// ============================================================================

struct FEngine3DContext
{
    // Camera
    FCamera3D MainCamera;
    
    // Renderer
    FRenderer3D Renderer;
    
    // Physics
    Physics::FPhysicsWorld PhysicsWorld;
    
    // Active scene
    std::vector<FModel3D> LoadedModels;
    std::vector<FLight> SceneLights;
    
    void Initialize(ECS::FWorld* World)
    {
        Renderer.Init();
        PhysicsWorld.Initialize(World);
        
        // Setup default light
        FLight Sun;
        Sun.Type = ELightType::Directional;
        Sun.Direction = Math::Vec3{-0.5f, -1.0f, -0.3f}.Normalized();
        Sun.Color = Math::Vec3{1.0f, 0.95f, 0.9f};
        Sun.Intensity = 1.5f;
        SceneLights.push_back(Sun);
    }
    
    void Shutdown()
    {
        Renderer.Shutdown();
        LoadedModels.clear();
        SceneLights.clear();
    }
    
    void Update(float DeltaTime)
    {
        MainCamera.Update(DeltaTime);
        PhysicsWorld.Step(DeltaTime);
    }
    
    void BeginRender()
    {
        Renderer.BeginFrame();
        Renderer.SetCamera(&MainCamera);
        Renderer.ClearLights();
        
        for (auto& Light : SceneLights)
        {
            Renderer.AddLight(Light);
        }
    }
    
    void EndRender()
    {
        Renderer.Render();
        Renderer.EndFrame();
    }
    
    FModel3D* LoadModel(const std::string& Path)
    {
        FModel3D Model;
        if (FGLTFLoader::Load(Path, Model))
        {
            LoadedModels.push_back(std::move(Model));
            return &LoadedModels.back();
        }
        return nullptr;
    }
};

// Global 3D context
inline FEngine3DContext* g_Engine3D = nullptr;

inline void Init3D(ECS::FWorld* World)
{
    if (!g_Engine3D)
    {
        g_Engine3D = new FEngine3DContext();
        g_Engine3D->Initialize(World);
    }
}

inline void Shutdown3D()
{
    if (g_Engine3D)
    {
        g_Engine3D->Shutdown();
        delete g_Engine3D;
        g_Engine3D = nullptr;
    }
}

inline FEngine3DContext* Get3D()
{
    return g_Engine3D;
}

// ============================================================================
// 3D Render System for ECS
// ============================================================================

class FRender3DSystem : public ECS::ISystem
{
public:
    void Init(ECS::FWorld& World) override
    {
        // Register components
    }
    
    void Update(ECS::FWorld& World, float DeltaTime) override
    {
        if (!g_Engine3D) return;
        
        auto& Renderer = g_Engine3D->Renderer;
        
        // Render all mesh components
        World.Each<FSkeletalMeshComponent, ECS::FTransformComponent>(
            [&](ECS::FEntityID Entity, FSkeletalMeshComponent& Mesh, ECS::FTransformComponent& Transform)
            {
                // Update animation
                Mesh.Update(DeltaTime);
                
                // Submit for rendering
                if (Mesh.Mesh)
                {
                    Math::Mat4 ModelMatrix = Math::Mat4::Translate(Transform.Position) *
                                              Math::Mat4::Scale(Transform.Scale);
                    
                    // Would submit with bone matrices
                    // For now, just render static mesh
                    // Renderer.Submit(Mesh.Mesh, Material, ModelMatrix);
                }
            }
        );
    }
    
    int GetPriority() const override { return 500; }
};

// ============================================================================
// Primitive Mesh Creation
// ============================================================================

namespace Primitives {

inline FMesh3D CreateCube(float Size = 1.0f)
{
    FMesh3D Mesh;
    float H = Size * 0.5f;
    
    // Positions
    Math::Vec3 Positions[] = {
        // Front
        {-H, -H,  H}, { H, -H,  H}, { H,  H,  H}, {-H,  H,  H},
        // Back
        { H, -H, -H}, {-H, -H, -H}, {-H,  H, -H}, { H,  H, -H},
        // Top
        {-H,  H,  H}, { H,  H,  H}, { H,  H, -H}, {-H,  H, -H},
        // Bottom
        {-H, -H, -H}, { H, -H, -H}, { H, -H,  H}, {-H, -H,  H},
        // Right
        { H, -H,  H}, { H, -H, -H}, { H,  H, -H}, { H,  H,  H},
        // Left
        {-H, -H, -H}, {-H, -H,  H}, {-H,  H,  H}, {-H,  H, -H}
    };
    
    Math::Vec3 Normals[] = {
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0},
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}
    };
    
    Math::Vec2 UVs[] = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };
    
    for (int i = 0; i < 24; i++)
    {
        FVertex3D V;
        V.Position = Positions[i];
        V.Normal = Normals[i];
        V.TexCoord = UVs[i];
        Mesh.Vertices.push_back(V);
    }
    
    uint32_t Indices[] = {
        0, 1, 2, 0, 2, 3,       // Front
        4, 5, 6, 4, 6, 7,       // Back
        8, 9, 10, 8, 10, 11,    // Top
        12, 13, 14, 12, 14, 15, // Bottom
        16, 17, 18, 16, 18, 19, // Right
        20, 21, 22, 20, 22, 23  // Left
    };
    
    for (int i = 0; i < 36; i++)
    {
        Mesh.Indices.push_back(Indices[i]);
    }
    
    Mesh.CalculateTangents();
    Mesh.CalculateBounds();
    
    FSubMesh Sub;
    Sub.IndexOffset = 0;
    Sub.IndexCount = 36;
    Sub.MaterialIndex = 0;
    Mesh.SubMeshes.push_back(Sub);
    
    return Mesh;
}

inline FMesh3D CreateSphere(float Radius = 0.5f, int Segments = 32, int Rings = 16)
{
    FMesh3D Mesh;
    
    for (int Ring = 0; Ring <= Rings; Ring++)
    {
        float Phi = Math::PI * Ring / Rings;
        float SinPhi = std::sin(Phi);
        float CosPhi = std::cos(Phi);
        
        for (int Seg = 0; Seg <= Segments; Seg++)
        {
            float Theta = Math::TAU * Seg / Segments;
            float SinTheta = std::sin(Theta);
            float CosTheta = std::cos(Theta);
            
            FVertex3D V;
            V.Normal = Math::Vec3{SinPhi * CosTheta, CosPhi, SinPhi * SinTheta};
            V.Position = V.Normal * Radius;
            V.TexCoord = Math::Vec2{
                static_cast<float>(Seg) / Segments,
                static_cast<float>(Ring) / Rings
            };
            
            Mesh.Vertices.push_back(V);
        }
    }
    
    for (int Ring = 0; Ring < Rings; Ring++)
    {
        for (int Seg = 0; Seg < Segments; Seg++)
        {
            int Current = Ring * (Segments + 1) + Seg;
            int Next = Current + Segments + 1;
            
            Mesh.Indices.push_back(Current);
            Mesh.Indices.push_back(Next);
            Mesh.Indices.push_back(Current + 1);
            
            Mesh.Indices.push_back(Current + 1);
            Mesh.Indices.push_back(Next);
            Mesh.Indices.push_back(Next + 1);
        }
    }
    
    Mesh.CalculateTangents();
    Mesh.CalculateBounds();
    
    FSubMesh Sub;
    Sub.IndexOffset = 0;
    Sub.IndexCount = static_cast<uint32_t>(Mesh.Indices.size());
    Sub.MaterialIndex = 0;
    Mesh.SubMeshes.push_back(Sub);
    
    return Mesh;
}

inline FMesh3D CreatePlane(float Width = 10.0f, float Height = 10.0f, int SegmentsX = 1, int SegmentsY = 1)
{
    FMesh3D Mesh;
    
    float HalfW = Width * 0.5f;
    float HalfH = Height * 0.5f;
    
    for (int Y = 0; Y <= SegmentsY; Y++)
    {
        for (int X = 0; X <= SegmentsX; X++)
        {
            float U = static_cast<float>(X) / SegmentsX;
            float V = static_cast<float>(Y) / SegmentsY;
            
            FVertex3D Vert;
            Vert.Position = Math::Vec3{
                U * Width - HalfW,
                0,
                V * Height - HalfH
            };
            Vert.Normal = Math::Vec3{0, 1, 0};
            Vert.TexCoord = Math::Vec2{U, V};
            Vert.Tangent = Math::Vec3{1, 0, 0};
            Vert.Bitangent = Math::Vec3{0, 0, 1};
            
            Mesh.Vertices.push_back(Vert);
        }
    }
    
    for (int Y = 0; Y < SegmentsY; Y++)
    {
        for (int X = 0; X < SegmentsX; X++)
        {
            int TL = Y * (SegmentsX + 1) + X;
            int TR = TL + 1;
            int BL = TL + SegmentsX + 1;
            int BR = BL + 1;
            
            Mesh.Indices.push_back(TL);
            Mesh.Indices.push_back(BL);
            Mesh.Indices.push_back(TR);
            
            Mesh.Indices.push_back(TR);
            Mesh.Indices.push_back(BL);
            Mesh.Indices.push_back(BR);
        }
    }
    
    Mesh.CalculateBounds();
    
    FSubMesh Sub;
    Sub.IndexOffset = 0;
    Sub.IndexCount = static_cast<uint32_t>(Mesh.Indices.size());
    Sub.MaterialIndex = 0;
    Mesh.SubMeshes.push_back(Sub);
    
    return Mesh;
}

inline FMesh3D CreateCapsule(float Radius = 0.5f, float Height = 2.0f, int Segments = 16, int Rings = 8)
{
    FMesh3D Mesh;
    
    float CylinderHeight = Height - 2.0f * Radius;
    float HalfCylinder = CylinderHeight * 0.5f;
    
    // Top hemisphere
    for (int Ring = 0; Ring <= Rings / 2; Ring++)
    {
        float Phi = Math::PI * 0.5f * Ring / (Rings / 2);
        float SinPhi = std::sin(Phi);
        float CosPhi = std::cos(Phi);
        
        for (int Seg = 0; Seg <= Segments; Seg++)
        {
            float Theta = Math::TAU * Seg / Segments;
            
            FVertex3D V;
            V.Normal = Math::Vec3{
                SinPhi * std::cos(Theta),
                CosPhi,
                SinPhi * std::sin(Theta)
            };
            V.Position = V.Normal * Radius + Math::Vec3{0, HalfCylinder, 0};
            V.TexCoord = Math::Vec2{
                static_cast<float>(Seg) / Segments,
                0.5f - Ring * 0.25f / (Rings / 2)
            };
            
            Mesh.Vertices.push_back(V);
        }
    }
    
    // Cylinder
    for (int Ring = 0; Ring <= 1; Ring++)
    {
        float Y = Ring == 0 ? HalfCylinder : -HalfCylinder;
        
        for (int Seg = 0; Seg <= Segments; Seg++)
        {
            float Theta = Math::TAU * Seg / Segments;
            
            FVertex3D V;
            V.Normal = Math::Vec3{std::cos(Theta), 0, std::sin(Theta)};
            V.Position = Math::Vec3{
                Radius * std::cos(Theta),
                Y,
                Radius * std::sin(Theta)
            };
            V.TexCoord = Math::Vec2{
                static_cast<float>(Seg) / Segments,
                0.25f + Ring * 0.5f
            };
            
            Mesh.Vertices.push_back(V);
        }
    }
    
    // Bottom hemisphere
    for (int Ring = 0; Ring <= Rings / 2; Ring++)
    {
        float Phi = Math::PI * 0.5f + Math::PI * 0.5f * Ring / (Rings / 2);
        float SinPhi = std::sin(Phi);
        float CosPhi = std::cos(Phi);
        
        for (int Seg = 0; Seg <= Segments; Seg++)
        {
            float Theta = Math::TAU * Seg / Segments;
            
            FVertex3D V;
            V.Normal = Math::Vec3{
                SinPhi * std::cos(Theta),
                CosPhi,
                SinPhi * std::sin(Theta)
            };
            V.Position = V.Normal * Radius + Math::Vec3{0, -HalfCylinder, 0};
            V.TexCoord = Math::Vec2{
                static_cast<float>(Seg) / Segments,
                0.75f + Ring * 0.25f / (Rings / 2)
            };
            
            Mesh.Vertices.push_back(V);
        }
    }
    
    // Generate indices
    int RowSize = Segments + 1;
    int TotalRings = Rings / 2 + 2 + Rings / 2;
    
    for (int Ring = 0; Ring < TotalRings; Ring++)
    {
        for (int Seg = 0; Seg < Segments; Seg++)
        {
            int Current = Ring * RowSize + Seg;
            int Next = Current + RowSize;
            
            Mesh.Indices.push_back(Current);
            Mesh.Indices.push_back(Next);
            Mesh.Indices.push_back(Current + 1);
            
            Mesh.Indices.push_back(Current + 1);
            Mesh.Indices.push_back(Next);
            Mesh.Indices.push_back(Next + 1);
        }
    }
    
    Mesh.CalculateTangents();
    Mesh.CalculateBounds();
    
    FSubMesh Sub;
    Sub.IndexOffset = 0;
    Sub.IndexCount = static_cast<uint32_t>(Mesh.Indices.size());
    Sub.MaterialIndex = 0;
    Mesh.SubMeshes.push_back(Sub);
    
    return Mesh;
}

} // namespace Primitives

} // namespace Titan

#endif // TITAN_3D_HPP


