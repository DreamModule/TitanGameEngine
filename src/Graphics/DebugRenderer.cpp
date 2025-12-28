#include “DebugRenderer.hpp”
#include “../Core/Logger.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <cmath>

#ifndef GL_LINES
#define GL_LINES 0x0001
#define GL_ARRAY_BUFFER 0x8892
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DEPTH_TEST 0x0B71
#endif

typedef void (APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void (APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, ptrdiff_t size, const void *data, GLenum usage);
typedef void (APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void (APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void (APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

namespace {
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;

```
bool g_glFunctionsLoaded = false;

void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;

    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");

    g_glFunctionsLoaded = true;
}
```

}

namespace Titan::Graphics {

DebugRenderer::DebugRenderer()
: shader(nullptr)
, vao(0)
, vbo(0)
, initialized(false)
, enabled(true) {
}

void DebugRenderer::Init() {
if (initialized) return;

```
LoadGLFunctions();

const char* vertexSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec4 aColor;
    
    uniform mat4 uView;
    uniform mat4 uProjection;
    
    out vec4 vColor;
    
    void main() {
        gl_Position = uProjection * uView * vec4(aPos, 1.0);
        vColor = aColor;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec4 vColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vColor;
    }
)";

shader = new Shader();
if (!shader->LoadFromSource(vertexSrc, fragmentSrc)) {
    Logger::Error("Failed to create debug renderer shader");
    delete shader;
    shader = nullptr;
    return;
}

InitBuffers();

initialized = true;
Logger::Info("DebugRenderer initialized");
```

}

void DebugRenderer::Shutdown() {
if (!initialized) return;

```
CleanupBuffers();

if (shader) {
    delete shader;
    shader = nullptr;
}

initialized = false;
Logger::Info("DebugRenderer shutdown");
```

}

void DebugRenderer::InitBuffers() {
if (!glGenVertexArrays || !glGenBuffers) return;

```
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);

glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, vbo);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

glBindVertexArray(0);
```

}

void DebugRenderer::CleanupBuffers() {
if (vao && glDeleteVertexArrays) {
glDeleteVertexArrays(1, &vao);
vao = 0;
}

```
if (vbo && glDeleteBuffers) {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
}
```

}

void DebugRenderer::DrawLine(const Vec3& start, const Vec3& end, const Color& color, float duration, bool depthTest) {
DebugLine line;
line.start = start;
line.end = end;
line.color = color;
line.duration = duration;
line.depthTest = depthTest;
lines.push_back(line);
}

void DebugRenderer::DrawRay(const Vec3& origin, const Vec3& direction, float length, const Color& color, float duration, bool depthTest) {
Vec3 end = origin + direction.Normalized() * length;
DrawLine(origin, end, color, duration, depthTest);

```
Vec3 right = direction.Cross(Vec3::Up()).Normalized() * (length * 0.1f);
Vec3 arrowPoint = origin + direction.Normalized() * (length * 0.9f);
DrawLine(end, arrowPoint + right, color, duration, depthTest);
DrawLine(end, arrowPoint - right, color, duration, depthTest);
```

}

void DebugRenderer::DrawSphere(const Vec3& center, float radius, const Color& color, float duration, bool depthTest) {
DebugSphere sphere;
sphere.center = center;
sphere.radius = radius;
sphere.color = color;
sphere.duration = duration;
sphere.depthTest = depthTest;
spheres.push_back(sphere);
}

void DebugRenderer::DrawBox(const Vec3& center, const Vec3& extents, const Quaternion& rotation, const Color& color, float duration, bool depthTest) {
DebugBox box;
box.center = center;
box.extents = extents;
box.rotation = rotation;
box.color = color;
box.duration = duration;
box.depthTest = depthTest;
boxes.push_back(box);
}

void DebugRenderer::DrawAABB(const Vec3& min, const Vec3& max, const Color& color, float duration, bool depthTest) {
Vec3 center = (min + max) * 0.5f;
Vec3 extents = (max - min) * 0.5f;
DrawBox(center, extents, Quaternion::Identity(), color, duration, depthTest);
}

void DebugRenderer::DrawCircle(const Vec3& center, const Vec3& normal, float radius, const Color& color, float duration, bool depthTest) {
const int segments = 32;
const float angleStep = 2.0f * 3.14159265f / segments;

```
Vec3 tangent = (std::abs(normal.y) < 0.9f) ? Vec3::Up() : Vec3::Right();
Vec3 bitangent = normal.Cross(tangent).Normalized();
tangent = bitangent.Cross(normal).Normalized();

Vec3 prevPoint = center + (tangent * radius);

for (int i = 1; i <= segments; ++i) {
    float angle = i * angleStep;
    float cosAngle = std::cos(angle);
    float sinAngle = std::sin(angle);

    Vec3 point = center + (tangent * cosAngle + bitangent * sinAngle) * radius;
    DrawLine(prevPoint, point, color, duration, depthTest);
    prevPoint = point;
}
```

}

void DebugRenderer::DrawCross(const Vec3& position, float size, const Color& color, float duration, bool depthTest) {
float halfSize = size * 0.5f;
DrawLine(position - Vec3(halfSize, 0, 0), position + Vec3(halfSize, 0, 0), color, duration, depthTest);
DrawLine(position - Vec3(0, halfSize, 0), position + Vec3(0, halfSize, 0), color, duration, depthTest);
DrawLine(position - Vec3(0, 0, halfSize), position + Vec3(0, 0, halfSize), color, duration, depthTest);
}

void DebugRenderer::DrawAxis(const Vec3& position, float size, float duration, bool depthTest) {
DrawLine(position, position + Vec3::Right() * size, Color::Red(), duration, depthTest);
DrawLine(position, position + Vec3::Up() * size, Color::Green(), duration, depthTest);
DrawLine(position, position + Vec3::Forward() * size, Color::Blue(), duration, depthTest);
}

void DebugRenderer::DrawFrustum(const Mat4& viewProjection, const Color& color, float duration, bool depthTest) {
Vec3 corners[8] = {
Vec3(-1, -1, -1), Vec3(1, -1, -1), Vec3(1, 1, -1), Vec3(-1, 1, -1),
Vec3(-1, -1, 1), Vec3(1, -1, 1), Vec3(1, 1, 1), Vec3(-1, 1, 1)
};

```
Mat4 invVP = viewProjection;

for (int i = 0; i < 8; ++i) {
    corners[i] = invVP * corners[i];
}

DrawLine(corners[0], corners[1], color, duration, depthTest);
DrawLine(corners[1], corners[2], color, duration, depthTest);
DrawLine(corners[2], corners[3], color, duration, depthTest);
DrawLine(corners[3], corners[0], color, duration, depthTest);

DrawLine(corners[4], corners[5], color, duration, depthTest);
DrawLine(corners[5], corners[6], color, duration, depthTest);
DrawLine(corners[6], corners[7], color, duration, depthTest);
DrawLine(corners[7], corners[4], color, duration, depthTest);

DrawLine(corners[0], corners[4], color, duration, depthTest);
DrawLine(corners[1], corners[5], color, duration, depthTest);
DrawLine(corners[2], corners[6], color, duration, depthTest);
DrawLine(corners[3], corners[7], color, duration, depthTest);
```

}

void DebugRenderer::DrawGrid(const Vec3& center, int gridSize, float cellSize, const Color& color, float duration) {
float halfSize = gridSize * cellSize * 0.5f;

```
for (int i = 0; i <= gridSize; ++i) {
    float offset = i * cellSize - halfSize;

    Vec3 start1 = center + Vec3(offset, 0, -halfSize);
    Vec3 end1 = center + Vec3(offset, 0, halfSize);
    DrawLine(start1, end1, color, duration, true);

    Vec3 start2 = center + Vec3(-halfSize, 0, offset);
    Vec3 end2 = center + Vec3(halfSize, 0, offset);
    DrawLine(start2, end2, color, duration, true);
}
```

}

void DebugRenderer::Render(const Mat4& view, const Mat4& projection) {
if (!initialized || !enabled || !shader) return;

```
RenderLines(view, projection);
RenderSpheres(view, projection);
RenderBoxes(view, projection);
```

}

void DebugRenderer::RenderLines(const Mat4& view, const Mat4& projection) {
if (lines.empty()) return;

```
std::vector<float> vertexData;
vertexData.reserve(lines.size() * 14);

for (auto& line : lines) {
    vertexData.push_back(line.start.x);
    vertexData.push_back(line.start.y);
    vertexData.push_back(line.start.z);
    vertexData.push_back(line.color.r);
    vertexData.push_back(line.color.g);
    vertexData.push_back(line.color.b);
    vertexData.push_back(line.color.a);

    vertexData.push_back(line.end.x);
    vertexData.push_back(line.end.y);
    vertexData.push_back(line.end.z);
    vertexData.push_back(line.color.r);
    vertexData.push_back(line.color.g);
    vertexData.push_back(line.color.b);
    vertexData.push_back(line.color.a);
}

shader->Bind();
shader->SetMat4("uView", view.m);
shader->SetMat4("uProjection", projection.m);

if (glBindVertexArray && glBindBuffer && glBufferData) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, lines.size() * 2);

    glBindVertexArray(0);
}

shader->Unbind();
```

}

void DebugRenderer::RenderSpheres(const Mat4& view, const Mat4& projection) {
for (auto& sphere : spheres) {
const int segments = 16;
DrawCircle(sphere.center, Vec3::Up(), sphere.radius, sphere.color, 0.0f, sphere.depthTest);
DrawCircle(sphere.center, Vec3::Right(), sphere.radius, sphere.color, 0.0f, sphere.depthTest);
DrawCircle(sphere.center, Vec3::Forward(), sphere.radius, sphere.color, 0.0f, sphere.depthTest);
}

```
RenderLines(view, projection);
```

}

void DebugRenderer::RenderBoxes(const Mat4& view, const Mat4& projection) {
for (auto& box : boxes) {
Vec3 corners[8];
Vec3 ex = box.extents;

```
    Vec3 localCorners[8] = {
        Vec3(-ex.x, -ex.y, -ex.z), Vec3(ex.x, -ex.y, -ex.z),
        Vec3(ex.x, ex.y, -ex.z), Vec3(-ex.x, ex.y, -ex.z),
        Vec3(-ex.x, -ex.y, ex.z), Vec3(ex.x, -ex.y, ex.z),
        Vec3(ex.x, ex.y, ex.z), Vec3(-ex.x, ex.y, ex.z)
    };

    for (int i = 0; i < 8; ++i) {
        corners[i] = box.center + (box.rotation * localCorners[i]);
    }

    DrawLine(corners[0], corners[1], box.color, 0.0f, box.depthTest);
    DrawLine(corners[1], corners[2], box.color, 0.0f, box.depthTest);
    DrawLine(corners[2], corners[3], box.color, 0.0f, box.depthTest);
    DrawLine(corners[3], corners[0], box.color, 0.0f, box.depthTest);

    DrawLine(corners[4], corners[5], box.color, 0.0f, box.depthTest);
    DrawLine(corners[5], corners[6], box.color, 0.0f, box.depthTest);
    DrawLine(corners[6], corners[7], box.color, 0.0f, box.depthTest);
    DrawLine(corners[7], corners[4], box.color, 0.0f, box.depthTest);

    DrawLine(corners[0], corners[4], box.color, 0.0f, box.depthTest);
    DrawLine(corners[1], corners[5], box.color, 0.0f, box.depthTest);
    DrawLine(corners[2], corners[6], box.color, 0.0f, box.depthTest);
    DrawLine(corners[3], corners[7], box.color, 0.0f, box.depthTest);
}

RenderLines(view, projection);
```

}

void DebugRenderer::Update(float deltaTime) {
lines.erase(std::remove_if(lines.begin(), lines.end(), [deltaTime](DebugLine& line) {
if (line.duration > 0.0f) {
line.duration -= deltaTime;
return line.duration <= 0.0f;
}
return line.duration == 0.0f;
}), lines.end());

```
spheres.erase(std::remove_if(spheres.begin(), spheres.end(), [deltaTime](DebugSphere& sphere) {
    if (sphere.duration > 0.0f) {
        sphere.duration -= deltaTime;
        return sphere.duration <= 0.0f;
    }
    return sphere.duration == 0.0f;
}), spheres.end());

boxes.erase(std::remove_if(boxes.begin(), boxes.end(), [deltaTime](DebugBox& box) {
    if (box.duration > 0.0f) {
        box.duration -= deltaTime;
        return box.duration <= 0.0f;
    }
    return box.duration == 0.0f;
}), boxes.end());
```

}

void DebugRenderer::Clear() {
lines.clear();
spheres.clear();
boxes.clear();
rays.clear();
}

}
