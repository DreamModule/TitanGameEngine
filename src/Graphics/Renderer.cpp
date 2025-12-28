#include “Renderer.hpp”
#include “../Core/Logger.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <cstring>

#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_TRIANGLES 0x0004
#define GL_UNSIGNED_INT 0x1405
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_BACK 0x0405
#endif

typedef void (APIENTRY *PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

namespace {
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = nullptr;
bool g_glFunctionsLoaded = false;

```
void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)wglGetProcAddress("glDrawElementsBaseVertex");
    g_glFunctionsLoaded = true;
}
```

}

namespace Titan::Graphics {

Mat4 Mat4::Identity() {
Mat4 result;
std::memset(result.m, 0, sizeof(result.m));
result.m[0] = result.m[5] = result.m[10] = result.m[15] = 1.0f;
return result;
}

Mat4 Mat4::Perspective(float fov, float aspect, float near, float far) {
Mat4 result;
std::memset(result.m, 0, sizeof(result.m));

```
float tanHalfFov = std::tan(fov * 0.5f * 3.14159265f / 180.0f);

result.m[0] = 1.0f / (aspect * tanHalfFov);
result.m[5] = 1.0f / tanHalfFov;
result.m[10] = -(far + near) / (far - near);
result.m[11] = -1.0f;
result.m[14] = -(2.0f * far * near) / (far - near);

return result;
```

}

Mat4 Mat4::Orthographic(float left, float right, float bottom, float top, float near, float far) {
Mat4 result = Identity();

```
result.m[0] = 2.0f / (right - left);
result.m[5] = 2.0f / (top - bottom);
result.m[10] = -2.0f / (far - near);
result.m[12] = -(right + left) / (right - left);
result.m[13] = -(top + bottom) / (top - bottom);
result.m[14] = -(far + near) / (far - near);

return result;
```

}

Mat4 Mat4::LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
Vec3 f = (target - eye).Normalized();
Vec3 s = f.Cross(up).Normalized();
Vec3 u = s.Cross(f);

```
Mat4 result = Identity();
result.m[0] = s.x;
result.m[4] = s.y;
result.m[8] = s.z;
result.m[1] = u.x;
result.m[5] = u.y;
result.m[9] = u.z;
result.m[2] = -f.x;
result.m[6] = -f.y;
result.m[10] = -f.z;
result.m[12] = -s.Dot(eye);
result.m[13] = -u.Dot(eye);
result.m[14] = f.Dot(eye);

return result;
```

}

Mat4 Mat4::Translation(const Vec3& pos) {
Mat4 result = Identity();
result.m[12] = pos.x;
result.m[13] = pos.y;
result.m[14] = pos.z;
return result;
}

Mat4 Mat4::Rotation(const Quaternion& q) {
Mat4 result = Identity();

```
float xx = q.x * q.x;
float yy = q.y * q.y;
float zz = q.z * q.z;
float xy = q.x * q.y;
float xz = q.x * q.z;
float yz = q.y * q.z;
float wx = q.w * q.x;
float wy = q.w * q.y;
float wz = q.w * q.z;

result.m[0] = 1.0f - 2.0f * (yy + zz);
result.m[1] = 2.0f * (xy + wz);
result.m[2] = 2.0f * (xz - wy);

result.m[4] = 2.0f * (xy - wz);
result.m[5] = 1.0f - 2.0f * (xx + zz);
result.m[6] = 2.0f * (yz + wx);

result.m[8] = 2.0f * (xz + wy);
result.m[9] = 2.0f * (yz - wx);
result.m[10] = 1.0f - 2.0f * (xx + yy);

return result;
```

}

Mat4 Mat4::Scale(const Vec3& scale) {
Mat4 result = Identity();
result.m[0] = scale.x;
result.m[5] = scale.y;
result.m[10] = scale.z;
return result;
}

Mat4 Mat4::TRS(const Vec3& pos, const Quaternion& rot, const Vec3& scale) {
return Translation(pos) * Rotation(rot) * Scale(scale);
}

Mat4 Mat4::operator*(const Mat4& other) const {
Mat4 result;

```
for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
        result.m[row * 4 + col] = 0.0f;
        for (int k = 0; k < 4; ++k) {
            result.m[row * 4 + col] += m[row * 4 + k] * other.m[k * 4 + col];
        }
    }
}

return result;
```

}

Vec3 Mat4::operator*(const Vec3& v) const {
Vec3 result;
result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
return result;
}

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
, cameraPosition(Vec3::Zero()) {
viewMatrix = Mat4::Identity();
projectionMatrix = Mat4::Identity();
viewProjectionMatrix = Mat4::Identity();
}

void Renderer::Init() {
if (initialized) return;

```
LoadGLFunctions();

glEnable(GL_DEPTH_TEST);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

initialized = true;
Logger::Info("Renderer initialized");
```

}

void Renderer::Shutdown() {
if (!initialized) return;

```
renderQueue.Clear();
batcher.Clear();

initialized = false;
Logger::Info("Renderer shutdown");
```

}

void Renderer::BeginFrame() {
ResetStats();

```
glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
glClearColor(clearR, clearG, clearB, clearA);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

}

void Renderer::EndFrame() {
UpdateStats();
}

void Renderer::BeginScene(const Camera& camera, const Transform& cameraTransform) {
if (inScene) {
Logger::Warning(“BeginScene called while already in scene”);
return;
}

```
inScene = true;
cameraPosition = cameraTransform.position;

Vec3 forward = cameraTransform.Forward();
Vec3 target = cameraPosition + forward;
viewMatrix = Mat4::LookAt(cameraPosition, target, Vec3::Up());

if (camera.projectionType == ProjectionType::Perspective) {
    float aspect = (float)viewportWidth / (float)viewportHeight;
    projectionMatrix = Mat4::Perspective(
        camera.fieldOfView,
        aspect,
        camera.nearClip,
        camera.farClip
    );
} else {
    float halfWidth = camera.orthographicSize * camera.aspectRatio * 0.5f;
    float halfHeight = camera.orthographicSize * 0.5f;
    projectionMatrix = Mat4::Orthographic(
        -halfWidth, halfWidth,
        -halfHeight, halfHeight,
        camera.nearClip,
        camera.farClip
    );
}

viewProjectionMatrix = projectionMatrix * viewMatrix;

if (settings.enableFrustumCulling) {
    frustum.ExtractFromMatrix(viewProjectionMatrix);
}

renderQueue.Clear();
batcher.Clear();

currentMaterial = nullptr;
currentShader = nullptr;
```

}

void Renderer::EndScene() {
if (!inScene) {
Logger::Warning(“EndScene called without BeginScene”);
return;
}

```
if (settings.sortTransparentObjects) {
    renderQueue.Sort(cameraPosition);
}

Flush();

inScene = false;
```

}

void Renderer::Submit(Mesh* mesh, Material* material, const Transform& transform) {
if (!mesh || !material) return;

```
Mat4 modelMatrix = Mat4::TRS(transform.position, transform.rotation, transform.scale);
Submit(mesh, material, modelMatrix);
```

}

void Renderer::Submit(Mesh* mesh, Material* material, const Mat4& modelMatrix) {
if (!mesh || !material) return;

```
if (mesh->GetSubMeshCount() > 0) {
    for (uint32_t i = 0; i < mesh->GetSubMeshCount(); ++i) {
        Submit(mesh, i, material, modelMatrix);
    }
} else {
    Submit(mesh, 0, material, modelMatrix);
}
```

}

void Renderer::Submit(Mesh* mesh, uint32_t subMeshIndex, Material* material, const Mat4& modelMatrix) {
if (!mesh || !material) return;

```
Vec3 position(modelMatrix.m[12], modelMatrix.m[13], modelMatrix.m[14]);

if (settings.enableFrustumCulling) {
    float radius = 5.0f;
    if (!frustum.TestSphere(position, radius)) {
        stats.entitiesCulled++;
        return;
    }
}

float distanceToCamera = (position - cameraPosition).LengthSquared();

renderQueue.Submit(mesh, material, modelMatrix, subMeshIndex, distanceToCamera);
```

}

void Renderer::Flush() {
if (renderQueue.GetCommandCount() == 0) return;

```
if (settings.enableBatching) {
    RenderBatched();
} else {
    for (auto& cmd : renderQueue.GetCommands()) {
        RenderCommand(cmd);
    }
}
```

}

void Renderer::RenderBatched() {
batcher.Clear();

```
for (auto& cmd : renderQueue.GetCommands()) {
    batcher.AddCommand(cmd);
}

for (auto& batch : batcher.GetBatches()) {
    if (!batch.material || !batch.mesh) continue;

    if (currentMaterial != batch.material) {
        if (currentMaterial) {
            currentMaterial->Unbind();
        }
        batch.material->Bind();
        batch.material->SetMat4("uView", viewMatrix.m);
        batch.material->SetMat4("uProjection", projectionMatrix.m);
        currentMaterial = batch.material;
        currentShader = batch.material->GetShader();
    }

    batch.mesh->Bind();

    for (uint32_t i = 0; i < batch.instanceCount; ++i) {
        if (currentShader) {
            currentShader->SetMat4("uModel", batch.modelMatrices[i].m);
        }

        if (batch.mesh->GetSubMeshCount() > 0) {
            batch.mesh->DrawSubMesh(batch.subMeshIndex);
        } else {
            batch.mesh->Draw();
        }

        stats.drawCalls++;
        stats.triangles += batch.mesh->GetIndexCount() / 3;
    }

    batch.mesh->Unbind();
}

if (currentMaterial) {
    currentMaterial->Unbind();
    currentMaterial = nullptr;
    currentShader = nullptr;
}

stats.batches = batcher.GetBatchCount();
stats.instancesRendered = batcher.GetTotalInstances();
```

}

void Renderer::RenderCommand(const RenderCommand& cmd) {
if (!cmd.material || !cmd.mesh) return;

```
if (currentMaterial != cmd.material) {
    if (currentMaterial) {
        currentMaterial->Unbind();
    }
    cmd.material->Bind();
    cmd.material->SetMat4("uView", viewMatrix.m);
    cmd.material->SetMat4("uProjection", projectionMatrix.m);
    currentMaterial = cmd.material;
    currentShader = cmd.material->GetShader();
}

if (currentShader) {
    currentShader->SetMat4("uModel", cmd.modelMatrix.m);
}

cmd.mesh->Bind();

if (cmd.mesh->GetSubMeshCount() > 0) {
    cmd.mesh->DrawSubMesh(cmd.subMeshIndex);
} else {
    cmd.mesh->Draw();
}

cmd.mesh->Unbind();

stats.drawCalls++;
stats.triangles += cmd.mesh->GetIndexCount() / 3;
```

}

bool Renderer::CullObject(const Vec3& position, float radius) {
if (!settings.enableFrustumCulling) return false;

```
float distanceSquared = (position - cameraPosition).LengthSquared();
if (distanceSquared > settings.cullDistance * settings.cullDistance) {
    return true;
}

return !frustum.TestSphere(position, radius);
```

}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
viewportX = x;
viewportY = y;
viewportWidth = width;
viewportHeight = height;
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
clearR = r;
clearG = g;
clearB = b;
clearA = a;
}

void Renderer::ResetStats() {
stats = RendererStats();
}

void Renderer::UpdateStats() {
stats.vertices = stats.triangles * 3;
}

}
