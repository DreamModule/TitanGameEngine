/**
 * Titan 3D Renderer
 * 
 * Modern 3D rendering system with:
 * - Forward rendering pipeline
 * - PBR-ready materials
 * - Multiple light types
 * - Shadow mapping
 * - Post-processing ready
 */

#ifndef TITAN_RENDERER_3D_HPP
#define TITAN_RENDERER_3D_HPP

#include "../Titan_Core.hpp"
#include "../Titan_Graphics.hpp"
#include "Camera3D.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace Titan {

// ============================================================================
// Light Types
// ============================================================================

enum class ELightType
{
    Directional,
    Point,
    Spot
};

struct FLight
{
    ELightType Type = ELightType::Point;
    
    // Transform
    Math::Vec3 Position{0, 5, 0};
    Math::Vec3 Direction{0, -1, 0};
    
    // Color and intensity
    Math::Vec3 Color{1, 1, 1};
    float Intensity = 1.0f;
    
    // Attenuation (for point/spot)
    float Range = 10.0f;
    float Constant = 1.0f;
    float Linear = 0.09f;
    float Quadratic = 0.032f;
    
    // Spotlight
    float InnerConeAngle = 12.5f; // degrees
    float OuterConeAngle = 17.5f; // degrees
    
    // Shadows
    bool bCastShadows = false;
    int ShadowMapSize = 1024;
    float ShadowBias = 0.005f;
    float ShadowNearPlane = 0.1f;
    float ShadowFarPlane = 50.0f;
};

// ============================================================================
// Material
// ============================================================================

struct FMaterial3D
{
    // Base color
    Math::Vec4 Albedo{1, 1, 1, 1};
    Graphics::TextureHandle AlbedoTexture;
    
    // PBR properties
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AO = 1.0f;
    
    // Textures
    Graphics::TextureHandle NormalMap;
    Graphics::TextureHandle MetallicRoughnessMap;
    Graphics::TextureHandle AOMap;
    Graphics::TextureHandle EmissiveMap;
    
    // Emissive
    Math::Vec3 EmissiveColor{0, 0, 0};
    float EmissiveStrength = 1.0f;
    
    // Rendering
    bool bDoubleSided = false;
    bool bTransparent = false;
    float AlphaCutoff = 0.5f;
    
    // Shader
    Graphics::ShaderHandle Shader;
};

// ============================================================================
// Mesh Data
// ============================================================================

struct FVertex3D
{
    Math::Vec3 Position;
    Math::Vec3 Normal;
    Math::Vec2 TexCoord;
    Math::Vec3 Tangent;
    Math::Vec3 Bitangent;
    
    // For skeletal animation
    int BoneIDs[4] = {-1, -1, -1, -1};
    float BoneWeights[4] = {0, 0, 0, 0};
};

struct FSubMesh
{
    uint32_t IndexOffset = 0;
    uint32_t IndexCount = 0;
    uint32_t MaterialIndex = 0;
};

struct FMesh3D
{
    std::vector<FVertex3D> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<FSubMesh> SubMeshes;
    
    // Bounds
    Math::Vec3 BoundsMin{0, 0, 0};
    Math::Vec3 BoundsMax{0, 0, 0};
    Math::Vec3 BoundsCenter{0, 0, 0};
    float BoundsRadius = 0.0f;
    
    // GPU buffers
    Graphics::BufferHandle VertexBuffer;
    Graphics::BufferHandle IndexBuffer;
    bool bUploaded = false;
    
    void CalculateBounds()
    {
        if (Vertices.empty()) return;
        
        BoundsMin = Vertices[0].Position;
        BoundsMax = Vertices[0].Position;
        
        for (const auto& V : Vertices)
        {
            BoundsMin.x = std::min(BoundsMin.x, V.Position.x);
            BoundsMin.y = std::min(BoundsMin.y, V.Position.y);
            BoundsMin.z = std::min(BoundsMin.z, V.Position.z);
            BoundsMax.x = std::max(BoundsMax.x, V.Position.x);
            BoundsMax.y = std::max(BoundsMax.y, V.Position.y);
            BoundsMax.z = std::max(BoundsMax.z, V.Position.z);
        }
        
        BoundsCenter = Math::Vec3{
            (BoundsMin.x + BoundsMax.x) * 0.5f,
            (BoundsMin.y + BoundsMax.y) * 0.5f,
            (BoundsMin.z + BoundsMax.z) * 0.5f
        };
        
        BoundsRadius = (BoundsMax - BoundsCenter).Length();
    }
    
    void CalculateTangents()
    {
        for (size_t i = 0; i < Indices.size(); i += 3)
        {
            uint32_t I0 = Indices[i];
            uint32_t I1 = Indices[i + 1];
            uint32_t I2 = Indices[i + 2];
            
            FVertex3D& V0 = Vertices[I0];
            FVertex3D& V1 = Vertices[I1];
            FVertex3D& V2 = Vertices[I2];
            
            Math::Vec3 Edge1 = V1.Position - V0.Position;
            Math::Vec3 Edge2 = V2.Position - V0.Position;
            
            float DeltaU1 = V1.TexCoord.x - V0.TexCoord.x;
            float DeltaV1 = V1.TexCoord.y - V0.TexCoord.y;
            float DeltaU2 = V2.TexCoord.x - V0.TexCoord.x;
            float DeltaV2 = V2.TexCoord.y - V0.TexCoord.y;
            
            float F = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1 + 0.0001f);
            
            Math::Vec3 Tangent{
                F * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x),
                F * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y),
                F * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z)
            };
            
            Math::Vec3 Bitangent{
                F * (-DeltaU2 * Edge1.x + DeltaU1 * Edge2.x),
                F * (-DeltaU2 * Edge1.y + DeltaU1 * Edge2.y),
                F * (-DeltaU2 * Edge1.z + DeltaU1 * Edge2.z)
            };
            
            V0.Tangent = V0.Tangent + Tangent;
            V1.Tangent = V1.Tangent + Tangent;
            V2.Tangent = V2.Tangent + Tangent;
            
            V0.Bitangent = V0.Bitangent + Bitangent;
            V1.Bitangent = V1.Bitangent + Bitangent;
            V2.Bitangent = V2.Bitangent + Bitangent;
        }
        
        for (auto& V : Vertices)
        {
            V.Tangent = V.Tangent.Normalized();
            V.Bitangent = V.Bitangent.Normalized();
        }
    }
};

// ============================================================================
// Render Command
// ============================================================================

struct FRenderCommand3D
{
    FMesh3D* Mesh = nullptr;
    FMaterial3D* Material = nullptr;
    Math::Mat4 ModelMatrix;
    uint32_t SubMeshIndex = 0;
    float DistanceToCamera = 0.0f;
    bool bCastShadow = true;
};

// ============================================================================
// Renderer Statistics
// ============================================================================

struct FRenderer3DStats
{
    uint32_t DrawCalls = 0;
    uint32_t Triangles = 0;
    uint32_t Vertices = 0;
    uint32_t EntitiesCulled = 0;
    uint32_t LightsActive = 0;
    float FrameTime = 0.0f;
};

// ============================================================================
// 3D Renderer Class
// ============================================================================

class FRenderer3D
{
public:
    // Settings
    struct FSettings
    {
        bool bEnableFrustumCulling = true;
        bool bEnableShadows = true;
        bool bEnableNormalMaps = true;
        float ShadowDistance = 50.0f;
        int MaxLights = 8;
        Math::Vec3 AmbientColor{0.03f, 0.03f, 0.03f};
    };

    FSettings Settings;
    FRenderer3DStats Stats;

private:
    // Shaders
    Graphics::ShaderHandle DefaultShader;
    Graphics::ShaderHandle ShadowShader;
    Graphics::ShaderHandle SkyboxShader;
    
    // Shadow mapping
    uint32_t ShadowMapFBO = 0;
    uint32_t ShadowMapTexture = 0;
    int ShadowMapSize = 2048;
    
    // Render queue
    std::vector<FRenderCommand3D> OpaqueQueue;
    std::vector<FRenderCommand3D> TransparentQueue;
    
    // Lights
    std::vector<FLight> Lights;
    
    // Active camera
    FCamera3D* ActiveCamera = nullptr;
    
    // White texture for materials without textures
    Graphics::TextureHandle WhiteTexture;
    
    bool bInitialized = false;

public:
    FRenderer3D() = default;

    void Init()
    {
        if (bInitialized) return;
        
        CreateDefaultShaders();
        CreateShadowMap();
        CreateDefaultTextures();
        
        bInitialized = true;
    }

    void Shutdown()
    {
        if (!bInitialized) return;
        
        // Cleanup would go here
        bInitialized = false;
    }

    void SetCamera(FCamera3D* Camera)
    {
        ActiveCamera = Camera;
    }

    void AddLight(const FLight& Light)
    {
        if (Lights.size() < static_cast<size_t>(Settings.MaxLights))
        {
            Lights.push_back(Light);
        }
    }

    void ClearLights()
    {
        Lights.clear();
    }

    void BeginFrame()
    {
        Stats = FRenderer3DStats();
        OpaqueQueue.clear();
        TransparentQueue.clear();
    }

    void Submit(FMesh3D* Mesh, FMaterial3D* Material, const Math::Mat4& ModelMatrix,
                uint32_t SubMeshIndex = 0, bool bCastShadow = true)
    {
        if (!Mesh || !Material || !ActiveCamera)
        {
            return;
        }

        // Frustum culling
        if (Settings.bEnableFrustumCulling)
        {
            Math::Vec3 WorldCenter{
                ModelMatrix.m[3][0] + Mesh->BoundsCenter.x,
                ModelMatrix.m[3][1] + Mesh->BoundsCenter.y,
                ModelMatrix.m[3][2] + Mesh->BoundsCenter.z
            };
            
            // Get approximate scale from matrix
            float ScaleX = std::sqrt(ModelMatrix.m[0][0] * ModelMatrix.m[0][0] + 
                                      ModelMatrix.m[0][1] * ModelMatrix.m[0][1] + 
                                      ModelMatrix.m[0][2] * ModelMatrix.m[0][2]);
            float WorldRadius = Mesh->BoundsRadius * ScaleX;
            
            if (!ActiveCamera->IsSphereVisible(WorldCenter, WorldRadius))
            {
                Stats.EntitiesCulled++;
                return;
            }
        }

        FRenderCommand3D Cmd;
        Cmd.Mesh = Mesh;
        Cmd.Material = Material;
        Cmd.ModelMatrix = ModelMatrix;
        Cmd.SubMeshIndex = SubMeshIndex;
        Cmd.bCastShadow = bCastShadow;
        
        // Calculate distance for sorting
        Math::Vec3 MeshPos{ModelMatrix.m[3][0], ModelMatrix.m[3][1], ModelMatrix.m[3][2]};
        Math::Vec3 CamPos = ActiveCamera->GetPosition();
        Cmd.DistanceToCamera = (MeshPos - CamPos).LengthSquared();

        if (Material->bTransparent)
        {
            TransparentQueue.push_back(Cmd);
        }
        else
        {
            OpaqueQueue.push_back(Cmd);
        }
    }

    void Render()
    {
        if (!ActiveCamera)
        {
            return;
        }

        Stats.LightsActive = static_cast<uint32_t>(Lights.size());

        // Shadow pass (if enabled)
        if (Settings.bEnableShadows)
        {
            RenderShadowPass();
        }

        // Main pass - opaque objects first
        RenderOpaquePass();

        // Transparent objects (back to front)
        RenderTransparentPass();
    }

    void EndFrame()
    {
        // Calculate frame statistics
    }

    const FRenderer3DStats& GetStats() const { return Stats; }

private:
    void CreateDefaultShaders()
    {
        // Vertex shader for 3D objects
        const char* VertexShaderSource = R"(
            #version 330 core
            
            layout(location = 0) in vec3 aPosition;
            layout(location = 1) in vec3 aNormal;
            layout(location = 2) in vec2 aTexCoord;
            layout(location = 3) in vec3 aTangent;
            layout(location = 4) in vec3 aBitangent;
            
            out vec3 vWorldPos;
            out vec3 vNormal;
            out vec2 vTexCoord;
            out vec3 vTangent;
            out vec3 vBitangent;
            out vec4 vShadowCoord;
            
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProjection;
            uniform mat4 uLightSpaceMatrix;
            
            void main()
            {
                vec4 worldPos = uModel * vec4(aPosition, 1.0);
                vWorldPos = worldPos.xyz;
                
                mat3 normalMatrix = transpose(inverse(mat3(uModel)));
                vNormal = normalize(normalMatrix * aNormal);
                vTangent = normalize(normalMatrix * aTangent);
                vBitangent = normalize(normalMatrix * aBitangent);
                
                vTexCoord = aTexCoord;
                vShadowCoord = uLightSpaceMatrix * worldPos;
                
                gl_Position = uProjection * uView * worldPos;
            }
        )";

        // Fragment shader with lighting
        const char* FragmentShaderSource = R"(
            #version 330 core
            
            in vec3 vWorldPos;
            in vec3 vNormal;
            in vec2 vTexCoord;
            in vec3 vTangent;
            in vec3 vBitangent;
            in vec4 vShadowCoord;
            
            out vec4 FragColor;
            
            // Material
            uniform vec4 uAlbedo;
            uniform float uMetallic;
            uniform float uRoughness;
            uniform float uAO;
            uniform vec3 uEmissive;
            
            uniform sampler2D uAlbedoMap;
            uniform sampler2D uNormalMap;
            uniform sampler2D uShadowMap;
            
            uniform bool uHasAlbedoMap;
            uniform bool uHasNormalMap;
            uniform bool uUseShadows;
            
            // Camera
            uniform vec3 uCameraPos;
            
            // Ambient
            uniform vec3 uAmbientColor;
            
            // Lights (max 8)
            #define MAX_LIGHTS 8
            uniform int uLightCount;
            uniform int uLightTypes[MAX_LIGHTS];
            uniform vec3 uLightPositions[MAX_LIGHTS];
            uniform vec3 uLightDirections[MAX_LIGHTS];
            uniform vec3 uLightColors[MAX_LIGHTS];
            uniform float uLightIntensities[MAX_LIGHTS];
            uniform float uLightRanges[MAX_LIGHTS];
            uniform float uLightInnerAngles[MAX_LIGHTS];
            uniform float uLightOuterAngles[MAX_LIGHTS];
            
            const float PI = 3.14159265359;
            
            float DistributionGGX(vec3 N, vec3 H, float roughness)
            {
                float a = roughness * roughness;
                float a2 = a * a;
                float NdotH = max(dot(N, H), 0.0);
                float NdotH2 = NdotH * NdotH;
                
                float nom = a2;
                float denom = (NdotH2 * (a2 - 1.0) + 1.0);
                denom = PI * denom * denom;
                
                return nom / max(denom, 0.0001);
            }
            
            float GeometrySchlickGGX(float NdotV, float roughness)
            {
                float r = (roughness + 1.0);
                float k = (r * r) / 8.0;
                
                float nom = NdotV;
                float denom = NdotV * (1.0 - k) + k;
                
                return nom / max(denom, 0.0001);
            }
            
            float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
            {
                float NdotV = max(dot(N, V), 0.0);
                float NdotL = max(dot(N, L), 0.0);
                float ggx2 = GeometrySchlickGGX(NdotV, roughness);
                float ggx1 = GeometrySchlickGGX(NdotL, roughness);
                
                return ggx1 * ggx2;
            }
            
            vec3 FresnelSchlick(float cosTheta, vec3 F0)
            {
                return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
            }
            
            float ShadowCalculation(vec4 fragPosLightSpace)
            {
                if (!uUseShadows) return 0.0;
                
                vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                projCoords = projCoords * 0.5 + 0.5;
                
                if (projCoords.z > 1.0) return 0.0;
                
                float closestDepth = texture(uShadowMap, projCoords.xy).r;
                float currentDepth = projCoords.z;
                
                float bias = 0.005;
                float shadow = 0.0;
                vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
                
                // PCF filtering
                for(int x = -1; x <= 1; ++x)
                {
                    for(int y = -1; y <= 1; ++y)
                    {
                        float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                    }
                }
                shadow /= 9.0;
                
                return shadow;
            }
            
            void main()
            {
                // Get albedo
                vec4 albedo = uAlbedo;
                if (uHasAlbedoMap)
                {
                    albedo *= texture(uAlbedoMap, vTexCoord);
                }
                
                // Get normal
                vec3 N = normalize(vNormal);
                if (uHasNormalMap)
                {
                    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), N);
                    vec3 normalMap = texture(uNormalMap, vTexCoord).rgb * 2.0 - 1.0;
                    N = normalize(TBN * normalMap);
                }
                
                vec3 V = normalize(uCameraPos - vWorldPos);
                
                // Calculate reflectance at normal incidence
                vec3 F0 = vec3(0.04);
                F0 = mix(F0, albedo.rgb, uMetallic);
                
                // Reflectance equation
                vec3 Lo = vec3(0.0);
                
                for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i)
                {
                    vec3 L;
                    float attenuation = 1.0;
                    
                    if (uLightTypes[i] == 0) // Directional
                    {
                        L = normalize(-uLightDirections[i]);
                    }
                    else if (uLightTypes[i] == 1) // Point
                    {
                        L = normalize(uLightPositions[i] - vWorldPos);
                        float distance = length(uLightPositions[i] - vWorldPos);
                        attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                        attenuation *= clamp(1.0 - distance / uLightRanges[i], 0.0, 1.0);
                    }
                    else // Spot
                    {
                        L = normalize(uLightPositions[i] - vWorldPos);
                        float distance = length(uLightPositions[i] - vWorldPos);
                        attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                        
                        float theta = dot(L, normalize(-uLightDirections[i]));
                        float epsilon = uLightInnerAngles[i] - uLightOuterAngles[i];
                        float intensity = clamp((theta - uLightOuterAngles[i]) / epsilon, 0.0, 1.0);
                        attenuation *= intensity;
                    }
                    
                    vec3 H = normalize(V + L);
                    vec3 radiance = uLightColors[i] * uLightIntensities[i] * attenuation;
                    
                    // Cook-Torrance BRDF
                    float NDF = DistributionGGX(N, H, uRoughness);
                    float G = GeometrySmith(N, V, L, uRoughness);
                    vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
                    
                    vec3 numerator = NDF * G * F;
                    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                    vec3 specular = numerator / denominator;
                    
                    vec3 kS = F;
                    vec3 kD = vec3(1.0) - kS;
                    kD *= 1.0 - uMetallic;
                    
                    float NdotL = max(dot(N, L), 0.0);
                    Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
                }
                
                // Shadow
                float shadow = ShadowCalculation(vShadowCoord);
                
                // Ambient
                vec3 ambient = uAmbientColor * albedo.rgb * uAO;
                
                // Final color
                vec3 color = ambient + Lo * (1.0 - shadow * 0.5);
                
                // Emissive
                color += uEmissive;
                
                // Tone mapping (Reinhard)
                color = color / (color + vec3(1.0));
                
                // Gamma correction
                color = pow(color, vec3(1.0 / 2.2));
                
                FragColor = vec4(color, albedo.a);
            }
        )";

        DefaultShader = Graphics::g_Device->CreateShader(VertexShaderSource, FragmentShaderSource);

        // Shadow map shader
        const char* ShadowVertexShader = R"(
            #version 330 core
            layout(location = 0) in vec3 aPosition;
            
            uniform mat4 uLightSpaceMatrix;
            uniform mat4 uModel;
            
            void main()
            {
                gl_Position = uLightSpaceMatrix * uModel * vec4(aPosition, 1.0);
            }
        )";

        const char* ShadowFragmentShader = R"(
            #version 330 core
            void main()
            {
                // Depth is written automatically
            }
        )";

        ShadowShader = Graphics::g_Device->CreateShader(ShadowVertexShader, ShadowFragmentShader);
    }

    void CreateShadowMap()
    {
        // Would use OpenGL framebuffer here
        // For now, shadow mapping would be implemented through the graphics device
    }

    void CreateDefaultTextures()
    {
        // Create 1x1 white texture
        uint8_t WhitePixel[4] = {255, 255, 255, 255};
        Graphics::TextureDesc Desc;
        Desc.width = 1;
        Desc.height = 1;
        Desc.format = Graphics::TextureFormat::RGBA8;
        Desc.filter = Graphics::TextureFilter::Nearest;
        Desc.data = WhitePixel;
        
        WhiteTexture = Graphics::g_Device->CreateTexture(Desc);
    }

    void RenderShadowPass()
    {
        // Find directional light for shadows
        FLight* ShadowLight = nullptr;
        for (auto& Light : Lights)
        {
            if (Light.Type == ELightType::Directional && Light.bCastShadows)
            {
                ShadowLight = &Light;
                break;
            }
        }

        if (!ShadowLight) return;

        // Bind shadow framebuffer
        // glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapFBO);
        // glViewport(0, 0, ShadowMapSize, ShadowMapSize);
        // glClear(GL_DEPTH_BUFFER_BIT);

        // Calculate light space matrix
        Math::Vec3 LightDir = ShadowLight->Direction.Normalized();
        Math::Vec3 LightPos = ActiveCamera->GetPosition() - LightDir * 25.0f;
        
        // Render shadow casters
        Graphics::g_Device->BindPipeline(ShadowShader);
        
        for (const auto& Cmd : OpaqueQueue)
        {
            if (!Cmd.bCastShadow) continue;
            
            UploadMesh(Cmd.Mesh);
            Graphics::g_Device->SetUniformMat4(ShadowShader, "uModel", Cmd.ModelMatrix);
            
            // Draw
            Graphics::g_Device->BindVertexBuffer(Cmd.Mesh->VertexBuffer);
            Graphics::g_Device->BindIndexBuffer(Cmd.Mesh->IndexBuffer);
            
            if (Cmd.SubMeshIndex < Cmd.Mesh->SubMeshes.size())
            {
                const auto& Sub = Cmd.Mesh->SubMeshes[Cmd.SubMeshIndex];
                Graphics::g_Device->DrawIndexed(Sub.IndexCount, Sub.IndexOffset);
            }
            else
            {
                Graphics::g_Device->DrawIndexed(static_cast<uint32_t>(Cmd.Mesh->Indices.size()), 0);
            }
        }

        // Unbind shadow framebuffer
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderOpaquePass()
    {
        Graphics::g_Device->BindPipeline(DefaultShader);
        
        // Set camera matrices
        Graphics::g_Device->SetUniformMat4(DefaultShader, "uView", ActiveCamera->ViewMatrix);
        Graphics::g_Device->SetUniformMat4(DefaultShader, "uProjection", ActiveCamera->ProjectionMatrix);
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uCameraPos", 
            Math::Vec4{ActiveCamera->GetPosition().x, ActiveCamera->GetPosition().y, 
                       ActiveCamera->GetPosition().z, 1.0f});
        
        // Set ambient
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uAmbientColor",
            Math::Vec4{Settings.AmbientColor.x, Settings.AmbientColor.y, Settings.AmbientColor.z, 1.0f});
        
        // Set lights
        Graphics::g_Device->SetUniformInt(DefaultShader, "uLightCount", static_cast<int32_t>(Lights.size()));
        
        for (size_t i = 0; i < Lights.size() && i < 8; i++)
        {
            SetLightUniform(i, Lights[i]);
        }
        
        // Render opaque objects
        for (const auto& Cmd : OpaqueQueue)
        {
            RenderCommand(Cmd);
        }
    }

    void RenderTransparentPass()
    {
        // Sort back to front
        std::sort(TransparentQueue.begin(), TransparentQueue.end(),
            [](const FRenderCommand3D& A, const FRenderCommand3D& B)
            {
                return A.DistanceToCamera > B.DistanceToCamera;
            });

        // Enable blending
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto& Cmd : TransparentQueue)
        {
            RenderCommand(Cmd);
        }

        // Disable blending
        // glDisable(GL_BLEND);
    }

    void RenderCommand(const FRenderCommand3D& Cmd)
    {
        UploadMesh(Cmd.Mesh);
        BindMaterial(Cmd.Material);
        
        Graphics::g_Device->SetUniformMat4(DefaultShader, "uModel", Cmd.ModelMatrix);
        
        Graphics::g_Device->BindVertexBuffer(Cmd.Mesh->VertexBuffer);
        Graphics::g_Device->BindIndexBuffer(Cmd.Mesh->IndexBuffer);
        
        uint32_t IndexCount, IndexOffset;
        if (Cmd.SubMeshIndex < Cmd.Mesh->SubMeshes.size())
        {
            const auto& Sub = Cmd.Mesh->SubMeshes[Cmd.SubMeshIndex];
            IndexCount = Sub.IndexCount;
            IndexOffset = Sub.IndexOffset;
        }
        else
        {
            IndexCount = static_cast<uint32_t>(Cmd.Mesh->Indices.size());
            IndexOffset = 0;
        }
        
        Graphics::g_Device->DrawIndexed(IndexCount, IndexOffset);
        
        Stats.DrawCalls++;
        Stats.Triangles += IndexCount / 3;
        Stats.Vertices += IndexCount;
    }

    void UploadMesh(FMesh3D* Mesh)
    {
        if (Mesh->bUploaded) return;
        
        // Create vertex buffer
        Graphics::BufferDesc VBDesc;
        VBDesc.type = Graphics::BufferType::Vertex;
        VBDesc.usage = Graphics::BufferUsage::Static;
        VBDesc.size = Mesh->Vertices.size() * sizeof(FVertex3D);
        VBDesc.data = Mesh->Vertices.data();
        VBDesc.layout = {
            {3, Graphics::AttributeType::Float, false},  // Position
            {3, Graphics::AttributeType::Float, false},  // Normal
            {2, Graphics::AttributeType::Float, false},  // TexCoord
            {3, Graphics::AttributeType::Float, false},  // Tangent
            {3, Graphics::AttributeType::Float, false}   // Bitangent
        };
        
        Mesh->VertexBuffer = Graphics::g_Device->CreateBuffer(VBDesc);
        
        // Create index buffer
        Graphics::BufferDesc IBDesc;
        IBDesc.type = Graphics::BufferType::Index;
        IBDesc.usage = Graphics::BufferUsage::Static;
        IBDesc.size = Mesh->Indices.size() * sizeof(uint32_t);
        IBDesc.data = Mesh->Indices.data();
        
        Mesh->IndexBuffer = Graphics::g_Device->CreateBuffer(IBDesc);
        
        Mesh->bUploaded = true;
    }

    void BindMaterial(FMaterial3D* Material)
    {
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uAlbedo", Material->Albedo);
        
        // PBR parameters
        Math::Vec4 PBRParams{Material->Metallic, Material->Roughness, Material->AO, 1.0f};
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uMetallic", 
            Math::Vec4{Material->Metallic, 0, 0, 0});
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uRoughness", 
            Math::Vec4{Material->Roughness, 0, 0, 0});
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uAO", 
            Math::Vec4{Material->AO, 0, 0, 0});
        Graphics::g_Device->SetUniformVec4(DefaultShader, "uEmissive",
            Math::Vec4{Material->EmissiveColor.x * Material->EmissiveStrength,
                       Material->EmissiveColor.y * Material->EmissiveStrength,
                       Material->EmissiveColor.z * Material->EmissiveStrength, 1.0f});
        
        // Textures
        if (Material->AlbedoTexture.IsValid())
        {
            Graphics::g_Device->BindTexture(Material->AlbedoTexture, 0);
            Graphics::g_Device->SetUniformInt(DefaultShader, "uAlbedoMap", 0);
            Graphics::g_Device->SetUniformInt(DefaultShader, "uHasAlbedoMap", 1);
        }
        else
        {
            Graphics::g_Device->BindTexture(WhiteTexture, 0);
            Graphics::g_Device->SetUniformInt(DefaultShader, "uHasAlbedoMap", 0);
        }
        
        if (Material->NormalMap.IsValid() && Settings.bEnableNormalMaps)
        {
            Graphics::g_Device->BindTexture(Material->NormalMap, 1);
            Graphics::g_Device->SetUniformInt(DefaultShader, "uNormalMap", 1);
            Graphics::g_Device->SetUniformInt(DefaultShader, "uHasNormalMap", 1);
        }
        else
        {
            Graphics::g_Device->SetUniformInt(DefaultShader, "uHasNormalMap", 0);
        }
    }

    void SetLightUniform(size_t Index, const FLight& Light)
    {
        char Buffer[64];
        
        snprintf(Buffer, 64, "uLightTypes[%zu]", Index);
        Graphics::g_Device->SetUniformInt(DefaultShader, Buffer, static_cast<int>(Light.Type));
        
        snprintf(Buffer, 64, "uLightPositions[%zu]", Index);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{Light.Position.x, Light.Position.y, Light.Position.z, 1.0f});
        
        snprintf(Buffer, 64, "uLightDirections[%zu]", Index);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{Light.Direction.x, Light.Direction.y, Light.Direction.z, 0.0f});
        
        snprintf(Buffer, 64, "uLightColors[%zu]", Index);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{Light.Color.x, Light.Color.y, Light.Color.z, 1.0f});
        
        snprintf(Buffer, 64, "uLightIntensities[%zu]", Index);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{Light.Intensity, 0, 0, 0});
        
        snprintf(Buffer, 64, "uLightRanges[%zu]", Index);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{Light.Range, 0, 0, 0});
        
        snprintf(Buffer, 64, "uLightInnerAngles[%zu]", Index);
        float InnerCos = std::cos(Light.InnerConeAngle * Math::DEG_TO_RAD);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{InnerCos, 0, 0, 0});
        
        snprintf(Buffer, 64, "uLightOuterAngles[%zu]", Index);
        float OuterCos = std::cos(Light.OuterConeAngle * Math::DEG_TO_RAD);
        Graphics::g_Device->SetUniformVec4(DefaultShader, Buffer,
            Math::Vec4{OuterCos, 0, 0, 0});
    }
};

} // namespace Titan

#endif // TITAN_RENDERER_3D_HPP


