#pragma once

#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/Camera.hpp”
#include “../ECS/Components/MeshRenderer.hpp”
#include <vector>
#include <cstdint>

namespace Titan::Graphics {

using namespace ECS::Components;

struct Mat4 {
float m[16];

```
static Mat4 Identity();
static Mat4 Perspective(float fov, float aspect, float near, float far);
static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);
static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
static Mat4 Translation(const Vec3& pos);
static Mat4 Rotation(const Quaternion& rot);
static Mat4 Scale(const Vec3& scale);
static Mat4 TRS(const Vec3& pos, const Quaternion& rot, const Vec3& scale);

Mat4 operator*(const Mat4& other) const;
Vec3 operator*(const Vec3& v) const;
```

};

struct RenderBatch {
uint32_t vao;
uint32_t vbo;
uint32_t ebo;
uint32_t instanceVBO;

```
uint32_t vertexCount;
uint32_t indexCount;
uint32_t instanceCount;

uint32_t textureId;
uint32_t shaderId;

std::vector<Mat4> modelMatrices;
std::vector<Color> tints;
```

};

class Renderer {
public:
static Renderer& Get() {
static Renderer instance;
return instance;
}

```
void Init();
void Shutdown();

void BeginFrame();
void EndFrame();

void BeginScene(const Mat4& view, const Mat4& projection);
void EndScene();

void Submit(const Vec3& position, const Quaternion& rotation, const Vec3& scale,
            uint32_t meshId, uint32_t textureId, const Color& tint);

void Flush();

void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void SetClearColor(float r, float g, float b, float a);

uint32_t CreateCubeMesh();
uint32_t CreateSphereMesh(uint32_t segments);
uint32_t CreatePlaneMesh();
uint32_t CreateQuadMesh();

const RendererStats& GetStats() const { return stats; }

struct RendererStats {
    uint32_t drawCalls = 0;
    uint32_t triangles = 0;
    uint32_t vertices = 0;
    uint32_t instances = 0;
} stats;
```

private:
Renderer() = default;
~Renderer() = default;

```
Renderer(const Renderer&) = delete;
Renderer& operator=(const Renderer&) = delete;

void InitShaders();
void InitMeshes();
void CreateBatch(uint32_t meshId, uint32_t textureId);

uint32_t defaultShader = 0;
Mat4 viewMatrix;
Mat4 projectionMatrix;

std::vector<RenderBatch> batches;

struct MeshData {
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    uint32_t vertexCount;
    uint32_t indexCount;
};

std::vector<MeshData> meshes;

bool initialized = false;
```

};

}
