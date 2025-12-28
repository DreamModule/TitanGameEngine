#pragma once

#include “Shader.hpp”
#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/MeshRenderer.hpp”
#include <vector>
#include <cstdint>

namespace Titan::Graphics {

using namespace ECS::Components;

struct DebugLine {
Vec3 start;
Vec3 end;
Color color;
float duration;
bool depthTest;
};

struct DebugSphere {
Vec3 center;
float radius;
Color color;
float duration;
bool depthTest;
};

struct DebugBox {
Vec3 center;
Vec3 extents;
Quaternion rotation;
Color color;
float duration;
bool depthTest;
};

struct DebugRay {
Vec3 origin;
Vec3 direction;
float length;
Color color;
float duration;
bool depthTest;
};

class DebugRenderer {
public:
static DebugRenderer& Get() {
static DebugRenderer instance;
return instance;
}

```
void Init();
void Shutdown();

void DrawLine(const Vec3& start, const Vec3& end, const Color& color = Color::White(), 
              float duration = 0.0f, bool depthTest = true);

void DrawRay(const Vec3& origin, const Vec3& direction, float length = 1.0f,
             const Color& color = Color::White(), float duration = 0.0f, bool depthTest = true);

void DrawSphere(const Vec3& center, float radius, const Color& color = Color::White(),
                float duration = 0.0f, bool depthTest = true);

void DrawBox(const Vec3& center, const Vec3& extents, const Quaternion& rotation = Quaternion::Identity(),
             const Color& color = Color::White(), float duration = 0.0f, bool depthTest = true);

void DrawAABB(const Vec3& min, const Vec3& max, const Color& color = Color::White(),
              float duration = 0.0f, bool depthTest = true);

void DrawCircle(const Vec3& center, const Vec3& normal, float radius,
                const Color& color = Color::White(), float duration = 0.0f, bool depthTest = true);

void DrawCross(const Vec3& position, float size = 1.0f, const Color& color = Color::White(),
               float duration = 0.0f, bool depthTest = true);

void DrawAxis(const Vec3& position, float size = 1.0f, float duration = 0.0f, bool depthTest = true);

void DrawFrustum(const Mat4& viewProjection, const Color& color = Color::Yellow(),
                 float duration = 0.0f, bool depthTest = true);

void DrawGrid(const Vec3& center, int gridSize, float cellSize, const Color& color = Color(0.5f, 0.5f, 0.5f, 0.5f),
              float duration = 0.0f);

void Render(const Mat4& view, const Mat4& projection);

void Update(float deltaTime);

void Clear();

void SetEnabled(bool enabled) { this->enabled = enabled; }
bool IsEnabled() const { return enabled; }
```

private:
DebugRenderer();
~DebugRenderer() = default;

```
DebugRenderer(const DebugRenderer&) = delete;
DebugRenderer& operator=(const DebugRenderer&) = delete;

void InitBuffers();
void CleanupBuffers();

void RenderLines(const Mat4& view, const Mat4& projection);
void RenderSpheres(const Mat4& view, const Mat4& projection);
void RenderBoxes(const Mat4& view, const Mat4& projection);

void GenerateSphereVertices(const Vec3& center, float radius, std::vector<Vec3>& vertices);
void GenerateBoxVertices(const Vec3& center, const Vec3& extents, const Quaternion& rotation, std::vector<Vec3>& vertices);

std::vector<DebugLine> lines;
std::vector<DebugSphere> spheres;
std::vector<DebugBox> boxes;
std::vector<DebugRay> rays;

Shader* shader;
uint32_t vao;
uint32_t vbo;
bool initialized;
bool enabled;
```

};

}
