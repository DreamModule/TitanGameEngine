#include “Renderer.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <cmath>
#include <cstring>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
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
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const char *const*string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const char *name);
typedef void (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY *PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

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
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;

```
void LoadGLFunctions() {
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
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

void Renderer::Init() {
if (initialized) return;

```
LoadGLFunctions();
InitShaders();
InitMeshes();

glEnable(GL_DEPTH_TEST);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

initialized = true;
```

}

void Renderer::Shutdown() {
if (!initialized) return;

```
for (auto& batch : batches) {
    if (batch.vao) glDeleteVertexArrays(1, &batch.vao);
    if (batch.vbo) glDeleteBuffers(1, &batch.vbo);
    if (batch.ebo) glDeleteBuffers(1, &batch.ebo);
    if (batch.instanceVBO) glDeleteBuffers(1, &batch.instanceVBO);
}
batches.clear();

for (auto& mesh : meshes) {
    if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
    if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    if (mesh.ebo) glDeleteBuffers(1, &mesh.ebo);
}
meshes.clear();

initialized = false;
```

}

void Renderer::BeginFrame() {
stats = RendererStats{};
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
Flush();
}

void Renderer::BeginScene(const Mat4& view, const Mat4& projection) {
viewMatrix = view;
projectionMatrix = projection;
batches.clear();
}

void Renderer::EndScene() {
Flush();
}

void Renderer::Submit(const Vec3& position, const Quaternion& rotation, const Vec3& scale,
uint32_t meshId, uint32_t textureId, const Color& tint) {

```
bool foundBatch = false;
for (auto& batch : batches) {
    if (batch.vao == meshId && batch.textureId == textureId) {
        batch.modelMatrices.push_back(Mat4::TRS(position, rotation, scale));
        batch.tints.push_back(tint);
        batch.instanceCount++;
        foundBatch = true;
        break;
    }
}

if (!foundBatch) {
    RenderBatch batch;
    batch.vao = meshId;
    batch.textureId = textureId;
    batch.shaderId = defaultShader;
    batch.instanceCount = 1;
    batch.modelMatrices.push_back(Mat4::TRS(position, rotation, scale));
    batch.tints.push_back(tint);
    batches.push_back(batch);
}
```

}

void Renderer::Flush() {
if (!glUseProgram || !glUniformMatrix4fv || !glUniform4f) return;

```
glUseProgram(defaultShader);

GLint viewLoc = glGetUniformLocation(defaultShader, "uView");
GLint projLoc = glGetUniformLocation(defaultShader, "uProjection");
GLint modelLoc = glGetUniformLocation(defaultShader, "uModel");
GLint colorLoc = glGetUniformLocation(defaultShader, "uColor");

if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix.m);
if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix.m);

for (auto& batch : batches) {
    if (batch.vao >= meshes.size()) continue;
    
    auto& mesh = meshes[batch.vao];
    glBindVertexArray(mesh.vao);
    
    for (size_t i = 0; i < batch.instanceCount; ++i) {
        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, batch.modelMatrices[i].m);
        if (colorLoc >= 0) {
            auto& c = batch.tints[i];
            glUniform4f(colorLoc, c.r, c.g, c.b, c.a);
        }
        
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        
        stats.drawCalls++;
        stats.triangles += mesh.indexCount / 3;
    }
}

glBindVertexArray(0);
batches.clear();
```

}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
glViewport(x, y, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
glClearColor(r, g, b, a);
}

void Renderer::InitShaders() {
const char* vertexSrc = R”(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

```
    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    
    out vec3 vNormal;
    
    void main() {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        vNormal = mat3(uModel) * aNormal;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec3 vNormal;
    out vec4 FragColor;
    
    uniform vec4 uColor;
    
    void main() {
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        float diff = max(dot(normalize(vNormal), lightDir), 0.0);
        vec3 ambient = vec3(0.3);
        vec3 result = (ambient + diff * 0.7) * uColor.rgb;
        FragColor = vec4(result, uColor.a);
    }
)";

GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertShader, 1, &vertexSrc, nullptr);
glCompileShader(vertShader);

GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(fragShader, 1, &fragmentSrc, nullptr);
glCompileShader(fragShader);

defaultShader = glCreateProgram();
glAttachShader(defaultShader, vertShader);
glAttachShader(defaultShader, fragShader);
glLinkProgram(defaultShader);

glDeleteShader(vertShader);
glDeleteShader(fragShader);
```

}

void Renderer::InitMeshes() {
CreateCubeMesh();
}

uint32_t Renderer::CreateCubeMesh() {
float vertices[] = {
-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

```
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
};

unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

MeshData mesh;
glGenVertexArrays(1, &mesh.vao);
glGenBuffers(1, &mesh.vbo);
glGenBuffers(1, &mesh.ebo);

glBindVertexArray(mesh.vao);

glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

glBindVertexArray(0);

mesh.vertexCount = 24;
mesh.indexCount = 36;

meshes.push_back(mesh);
return meshes.size() - 1;
```

}

uint32_t Renderer::CreateSphereMesh(uint32_t segments) {
return 0;
}

uint32_t Renderer::CreatePlaneMesh() {
return 0;
}

uint32_t Renderer::CreateQuadMesh() {
return 0;
}

}
