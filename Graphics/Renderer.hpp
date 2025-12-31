#pragma once

#include “Material.hpp”
#include “Mesh.hpp”
#include “RenderQueue.hpp”
#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/Camera.hpp”
#include <cstdint>

namespace Titan::Graphics {

using namespace ECS::Components;

struct RendererStats {
uint32_t drawCalls = 0;
uint32_t triangles = 0;
uint32_t vertices = 0;
uint32_t batches = 0;
uint32_t instancesRendered = 0;
uint32_t entitiesCulled = 0;
float frameTime = 0.0f;
};

struct RenderSettings {
bool enableFrustumCulling = true;
bool enableBatching = true;
bool enableWireframe = false;
bool enableDepthPrepass = false;
bool sortTransparentObjects = true;
float cullDistance = 1000.0f;
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

void BeginScene(const Camera& camera, const Transform& cameraTransform);
void EndScene();

void Submit(Mesh* mesh, Material* material, const Transform& transform);
void Submit(Mesh* mesh, Material* material, const Mat4& modelMatrix);
void Submit(Mesh* mesh, uint32_t subMeshIndex, Material* material, const Mat4& modelMatrix);

void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void SetClearColor(float r, float g, float b, float a);

const RendererStats& GetStats() const { return stats; }
void ResetStats();

RenderSettings& GetSettings() { return settings; }
const RenderSettings& GetSettings() const { return settings; }

const Mat4& GetViewMatrix() const { return viewMatrix; }
const Mat4& GetProjectionMatrix() const { return projectionMatrix; }
const Vec3& GetCameraPosition() const { return cameraPosition; }
```

private:
Renderer();
~Renderer() = default;

```
Renderer(const Renderer&) = delete;
Renderer& operator=(const Renderer&) = delete;

void Flush();
void RenderBatched();
void RenderCommand(const RenderCommand& cmd);
bool CullObject(const Vec3& position, float radius);

void UpdateStats();

Mat4 viewMatrix;
Mat4 projectionMatrix;
Mat4 viewProjectionMatrix;
Vec3 cameraPosition;

RenderQueue renderQueue;
RenderBatcher batcher;
Frustum frustum;

RendererStats stats;
RenderSettings settings;

bool initialized;
bool inScene;

Material* currentMaterial;
Shader* currentShader;

uint32_t viewportX;
uint32_t viewportY;
uint32_t viewportWidth;
uint32_t viewportHeight;

float clearR;
float clearG;
float clearB;
float clearA;
```

};

}
