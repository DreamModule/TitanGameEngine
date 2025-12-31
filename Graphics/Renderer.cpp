/**
 * Titan Renderer Implementation
 */

#include "Renderer.hpp"
#include "../Core/Logger.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <gl/GL.h>
#endif

#include <cstring>
#include <cstdio>

#ifndef GL_DEPTH_BUFFER_BIT
    #define GL_DEPTH_BUFFER_BIT 0x00000100
    #define GL_COLOR_BUFFER_BIT 0x00004000
    #define GL_TRIANGLES 0x0004
    #define GL_UNSIGNED_INT 0x1405
    #define GL_DEPTH_TEST 0x0B71
    #define GL_CULL_FACE 0x0B44
    #define GL_BACK 0x0405
#endif

// OpenGL function pointer type
typedef void (APIENTRY* PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum, GLsizei, GLenum, const void*, GLint);

namespace {

PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = nullptr;
bool GGLFunctionsLoaded = false;

void LoadGLFunctions()
{
    if (GGLFunctionsLoaded) return;
    
#ifdef _WIN32
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)wglGetProcAddress("glDrawElementsBaseVertex");
    
    if (!glDrawElementsBaseVertex)
    {
        fprintf(stderr, "[Renderer] Warning: glDrawElementsBaseVertex not available\n");
    }
#endif
    
    GGLFunctionsLoaded = true;
}

void CheckGLError(const char* Operation)
{
    GLenum Error = glGetError();
    if (Error != GL_NO_ERROR)
    {
        fprintf(stderr, "[Renderer] OpenGL Error during %s: 0x%X\n", Operation, Error);
    }
}

} // anonymous namespace

namespace Titan::Graphics {

// ============================================================================
// Mat4 Implementation
// ============================================================================

Mat4 Mat4::Identity()
{
    Mat4 Result;
    std::memset(Result.m, 0, sizeof(Result.m));
    Result.m[0] = Result.m[5] = Result.m[10] = Result.m[15] = 1.0f;
    return Result;
}

Mat4 Mat4::Perspective(float FOV, float Aspect, float Near, float Far)
{
    Mat4 Result;
    std::memset(Result.m, 0, sizeof(Result.m));

    const float PI = 3.14159265f;
    float TanHalfFOV = std::tan(FOV * 0.5f * PI / 180.0f);

    if (TanHalfFOV == 0.0f || Aspect == 0.0f || (Far - Near) == 0.0f)
    {
        fprintf(stderr, "[Renderer] Invalid perspective parameters\n");
        return Identity();
    }

    Result.m[0] = 1.0f / (Aspect * TanHalfFOV);
    Result.m[5] = 1.0f / TanHalfFOV;
    Result.m[10] = -(Far + Near) / (Far - Near);
    Result.m[11] = -1.0f;
    Result.m[14] = -(2.0f * Far * Near) / (Far - Near);

    return Result;
}

Mat4 Mat4::Orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    Mat4 Result = Identity();

    float Width = Right - Left;
    float Height = Top - Bottom;
    float Depth = Far - Near;

    if (Width == 0.0f || Height == 0.0f || Depth == 0.0f)
    {
        fprintf(stderr, "[Renderer] Invalid orthographic parameters\n");
        return Identity();
    }

    Result.m[0] = 2.0f / Width;
    Result.m[5] = 2.0f / Height;
    Result.m[10] = -2.0f / Depth;
    Result.m[12] = -(Right + Left) / Width;
    Result.m[13] = -(Top + Bottom) / Height;
    Result.m[14] = -(Far + Near) / Depth;

    return Result;
}

Mat4 Mat4::LookAt(const Vec3& Eye, const Vec3& Target, const Vec3& Up)
{
    Vec3 F = (Target - Eye).Normalized();
    Vec3 S = F.Cross(Up).Normalized();
    Vec3 U = S.Cross(F);

    Mat4 Result = Identity();
    Result.m[0] = S.x;
    Result.m[4] = S.y;
    Result.m[8] = S.z;
    Result.m[1] = U.x;
    Result.m[5] = U.y;
    Result.m[9] = U.z;
    Result.m[2] = -F.x;
    Result.m[6] = -F.y;
    Result.m[10] = -F.z;
    Result.m[12] = -S.Dot(Eye);
    Result.m[13] = -U.Dot(Eye);
    Result.m[14] = F.Dot(Eye);

    return Result;
}

Mat4 Mat4::Translation(const Vec3& Position)
{
    Mat4 Result = Identity();
    Result.m[12] = Position.x;
    Result.m[13] = Position.y;
    Result.m[14] = Position.z;
    return Result;
}

Mat4 Mat4::Rotation(const Quaternion& Q)
{
    Mat4 Result = Identity();

    float XX = Q.x * Q.x;
    float YY = Q.y * Q.y;
    float ZZ = Q.z * Q.z;
    float XY = Q.x * Q.y;
    float XZ = Q.x * Q.z;
    float YZ = Q.y * Q.z;
    float WX = Q.w * Q.x;
    float WY = Q.w * Q.y;
    float WZ = Q.w * Q.z;

    Result.m[0] = 1.0f - 2.0f * (YY + ZZ);
    Result.m[1] = 2.0f * (XY + WZ);
    Result.m[2] = 2.0f * (XZ - WY);

    Result.m[4] = 2.0f * (XY - WZ);
    Result.m[5] = 1.0f - 2.0f * (XX + ZZ);
    Result.m[6] = 2.0f * (YZ + WX);

    Result.m[8] = 2.0f * (XZ + WY);
    Result.m[9] = 2.0f * (YZ - WX);
    Result.m[10] = 1.0f - 2.0f * (XX + YY);

    return Result;
}

Mat4 Mat4::Scale(const Vec3& ScaleVec)
{
    Mat4 Result = Identity();
    Result.m[0] = ScaleVec.x;
    Result.m[5] = ScaleVec.y;
    Result.m[10] = ScaleVec.z;
    return Result;
}

Mat4 Mat4::TRS(const Vec3& Position, const Quaternion& Rotation, const Vec3& ScaleVec)
{
    return Translation(Position) * Mat4::Rotation(Rotation) * Scale(ScaleVec);
}

Mat4 Mat4::operator*(const Mat4& Other) const
{
    Mat4 Result;

    for (int Row = 0; Row < 4; ++Row)
    {
        for (int Col = 0; Col < 4; ++Col)
        {
            Result.m[Row * 4 + Col] = 0.0f;
            for (int K = 0; K < 4; ++K)
            {
                Result.m[Row * 4 + Col] += m[Row * 4 + K] * Other.m[K * 4 + Col];
            }
        }
    }

    return Result;
}

Vec3 Mat4::operator*(const Vec3& V) const
{
    Vec3 Result;
    Result.x = m[0] * V.x + m[4] * V.y + m[8] * V.z + m[12];
    Result.y = m[1] * V.x + m[5] * V.y + m[9] * V.z + m[13];
    Result.z = m[2] * V.x + m[6] * V.y + m[10] * V.z + m[14];
    return Result;
}

// ============================================================================
// Renderer Implementation
// ============================================================================

Renderer::Renderer()
    : initialized(false)
    , inScene(false)
    , currentMaterial(nullptr)
    , currentShader(nullptr)
    , viewportX(0)
    , viewportY(0)
    , viewportWidth(1280)
    , viewportHeight(720)
    , clearR(0.1f)
    , clearG(0.1f)
    , clearB(0.12f)
    , clearA(1.0f)
    , cameraPosition(Vec3::Zero())
{
    viewMatrix = Mat4::Identity();
    projectionMatrix = Mat4::Identity();
    viewProjectionMatrix = Mat4::Identity();
}

void Renderer::Init()
{
    if (initialized)
    {
        return;
    }

    LoadGLFunctions();

    glEnable(GL_DEPTH_TEST);
    CheckGLError("glEnable(GL_DEPTH_TEST)");
    
    glEnable(GL_CULL_FACE);
    CheckGLError("glEnable(GL_CULL_FACE)");
    
    glCullFace(GL_BACK);
    CheckGLError("glCullFace(GL_BACK)");

    initialized = true;
    Logger::Info("Renderer initialized");
}

void Renderer::Shutdown()
{
    if (!initialized)
    {
        return;
    }

    renderQueue.Clear();
    batcher.Clear();

    initialized = false;
    Logger::Info("Renderer shutdown");
}

void Renderer::BeginFrame()
{
    ResetStats();

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    glClearColor(clearR, clearG, clearB, clearA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    CheckGLError("BeginFrame");
}

void Renderer::EndFrame()
{
    UpdateStats();
    CheckGLError("EndFrame");
}

void Renderer::BeginScene(const Camera& InCamera, const Transform& CameraTransform)
{
    if (inScene)
    {
        Logger::Warning("BeginScene called while already in scene");
        return;
    }

    inScene = true;
    cameraPosition = CameraTransform.position;

    Vec3 Forward = CameraTransform.Forward();
    Vec3 Target = cameraPosition + Forward;
    viewMatrix = Mat4::LookAt(cameraPosition, Target, Vec3::Up());

    if (InCamera.projectionType == ProjectionType::Perspective)
    {
        float Aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
        projectionMatrix = Mat4::Perspective(
            InCamera.fieldOfView,
            Aspect,
            InCamera.nearClip,
            InCamera.farClip
        );
    }
    else
    {
        float HalfWidth = InCamera.orthographicSize * InCamera.aspectRatio * 0.5f;
        float HalfHeight = InCamera.orthographicSize * 0.5f;
        projectionMatrix = Mat4::Orthographic(
            -HalfWidth, HalfWidth,
            -HalfHeight, HalfHeight,
            InCamera.nearClip,
            InCamera.farClip
        );
    }

    viewProjectionMatrix = projectionMatrix * viewMatrix;

    if (settings.enableFrustumCulling)
    {
        frustum.ExtractFromMatrix(viewProjectionMatrix);
    }

    renderQueue.Clear();
    batcher.Clear();

    currentMaterial = nullptr;
    currentShader = nullptr;
}

void Renderer::EndScene()
{
    if (!inScene)
    {
        Logger::Warning("EndScene called without BeginScene");
        return;
    }

    if (settings.sortTransparentObjects)
    {
        renderQueue.Sort(cameraPosition);
    }

    Flush();

    inScene = false;
}

void Renderer::Submit(Mesh* InMesh, Material* InMaterial, const Transform& InTransform)
{
    if (!InMesh || !InMaterial)
    {
        return;
    }

    Mat4 ModelMatrix = Mat4::TRS(InTransform.position, InTransform.rotation, InTransform.scale);
    Submit(InMesh, InMaterial, ModelMatrix);
}

void Renderer::Submit(Mesh* InMesh, Material* InMaterial, const Mat4& ModelMatrix)
{
    if (!InMesh || !InMaterial)
    {
        return;
    }

    if (InMesh->GetSubMeshCount() > 0)
    {
        for (uint32_t i = 0; i < InMesh->GetSubMeshCount(); ++i)
        {
            Submit(InMesh, i, InMaterial, ModelMatrix);
        }
    }
    else
    {
        Submit(InMesh, 0, InMaterial, ModelMatrix);
    }
}

void Renderer::Submit(Mesh* InMesh, uint32_t SubMeshIndex, Material* InMaterial, const Mat4& ModelMatrix)
{
    if (!InMesh || !InMaterial)
    {
        return;
    }

    Vec3 Position(ModelMatrix.m[12], ModelMatrix.m[13], ModelMatrix.m[14]);

    if (settings.enableFrustumCulling)
    {
        float Radius = 5.0f; // TODO: Get from mesh bounds
        if (!frustum.TestSphere(Position, Radius))
        {
            stats.entitiesCulled++;
            return;
        }
    }

    float DistanceToCamera = (Position - cameraPosition).LengthSquared();

    renderQueue.Submit(InMesh, InMaterial, ModelMatrix, SubMeshIndex, DistanceToCamera);
}

void Renderer::Flush()
{
    if (renderQueue.GetCommandCount() == 0)
    {
        return;
    }

    if (settings.enableBatching)
    {
        RenderBatched();
    }
    else
    {
        for (auto& Cmd : renderQueue.GetCommands())
        {
            RenderCommand(Cmd);
        }
    }
}

void Renderer::RenderBatched()
{
    batcher.Clear();

    for (auto& Cmd : renderQueue.GetCommands())
    {
        batcher.AddCommand(Cmd);
    }

    for (auto& Batch : batcher.GetBatches())
    {
        if (!Batch.material || !Batch.mesh)
        {
            continue;
        }

        if (currentMaterial != Batch.material)
        {
            if (currentMaterial)
            {
                currentMaterial->Unbind();
            }
            Batch.material->Bind();
            Batch.material->SetMat4("uView", viewMatrix.m);
            Batch.material->SetMat4("uProjection", projectionMatrix.m);
            currentMaterial = Batch.material;
            currentShader = Batch.material->GetShader();
        }

        Batch.mesh->Bind();

        for (uint32_t i = 0; i < Batch.instanceCount; ++i)
        {
            if (currentShader)
            {
                currentShader->SetMat4("uModel", Batch.modelMatrices[i].m);
            }

            if (Batch.mesh->GetSubMeshCount() > 0)
            {
                Batch.mesh->DrawSubMesh(Batch.subMeshIndex);
            }
            else
            {
                Batch.mesh->Draw();
            }

            stats.drawCalls++;
            stats.triangles += Batch.mesh->GetIndexCount() / 3;
        }

        Batch.mesh->Unbind();
    }

    if (currentMaterial)
    {
        currentMaterial->Unbind();
        currentMaterial = nullptr;
        currentShader = nullptr;
    }

    stats.batches = batcher.GetBatchCount();
    stats.instancesRendered = batcher.GetTotalInstances();
}

void Renderer::RenderCommand(const RenderCommand& Cmd)
{
    if (!Cmd.material || !Cmd.mesh)
    {
        return;
    }

    if (currentMaterial != Cmd.material)
    {
        if (currentMaterial)
        {
            currentMaterial->Unbind();
        }
        Cmd.material->Bind();
        Cmd.material->SetMat4("uView", viewMatrix.m);
        Cmd.material->SetMat4("uProjection", projectionMatrix.m);
        currentMaterial = Cmd.material;
        currentShader = Cmd.material->GetShader();
    }

    if (currentShader)
    {
        currentShader->SetMat4("uModel", Cmd.modelMatrix.m);
    }

    Cmd.mesh->Bind();

    if (Cmd.mesh->GetSubMeshCount() > 0)
    {
        Cmd.mesh->DrawSubMesh(Cmd.subMeshIndex);
    }
    else
    {
        Cmd.mesh->Draw();
    }

    Cmd.mesh->Unbind();

    stats.drawCalls++;
    stats.triangles += Cmd.mesh->GetIndexCount() / 3;
}

bool Renderer::CullObject(const Vec3& Position, float Radius)
{
    if (!settings.enableFrustumCulling)
    {
        return false;
    }

    float DistanceSquared = (Position - cameraPosition).LengthSquared();
    if (DistanceSquared > settings.cullDistance * settings.cullDistance)
    {
        return true;
    }

    return !frustum.TestSphere(Position, Radius);
}

void Renderer::SetViewport(uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height)
{
    viewportX = X;
    viewportY = Y;
    viewportWidth = Width;
    viewportHeight = Height;
}

void Renderer::SetClearColor(float R, float G, float B, float A)
{
    clearR = R;
    clearG = G;
    clearB = B;
    clearA = A;
}

void Renderer::ResetStats()
{
    stats = RendererStats();
}

void Renderer::UpdateStats()
{
    stats.vertices = stats.triangles * 3;
}

} // namespace Titan::Graphics
