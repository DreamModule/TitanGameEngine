#include “Mesh.hpp”
#include “../Core/Logger.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
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
typedef void (APIENTRY *PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (APIENTRY *PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

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
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = nullptr;

```
bool g_glFunctionsLoaded = false;

void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;
    
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYSPROC)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)wglGetProcAddress("glDrawElementsBaseVertex");
    
    g_glFunctionsLoaded = true;
}
```

}

namespace Titan::Graphics {

Mesh::Mesh() : vao(0), vbo(0), ebo(0), vertexCount(0), indexCount(0) {
LoadGLFunctions();
}

Mesh::~Mesh() {
Cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept
: vao(other.vao), vbo(other.vbo), ebo(other.ebo),
vertexCount(other.vertexCount), indexCount(other.indexCount),
subMeshes(std::move(other.subMeshes)) {
other.vao = 0;
other.vbo = 0;
other.ebo = 0;
other.vertexCount = 0;
other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
if (this != &other) {
Cleanup();

```
    vao = other.vao;
    vbo = other.vbo;
    ebo = other.ebo;
    vertexCount = other.vertexCount;
    indexCount = other.indexCount;
    subMeshes = std::move(other.subMeshes);
    
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.vertexCount = 0;
    other.indexCount = 0;
}
return *this;
```

}

bool Mesh::LoadFromFile(const std::string& path) {
size_t dotPos = path.find_last_of(’.’);
if (dotPos == std::string::npos) {
Logger::Error(“Mesh file has no extension: “ + path);
return false;
}

```
std::string ext = path.substr(dotPos + 1);
std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

if (ext == "obj") return LoadOBJ(path);
if (ext == "fbx") return LoadFBX(path);
if (ext == "gltf" || ext == "glb") return LoadGLTF(path);

Logger::Error("Unsupported mesh format: " + ext);
return false;
```

}

bool Mesh::LoadFromData(const MeshData& data) {
if (data.vertices.empty()) {
Logger::Error(“Cannot create mesh from empty vertex data”);
return false;
}

```
std::vector<Vertex> vertices = data.vertices;

if (!data.indices.empty()) {
    CalculateTangents(vertices, data.indices);
}

SetupMesh(vertices, data.indices);
subMeshes = data.subMeshes;

if (subMeshes.empty() && !data.indices.empty()) {
    SubMesh submesh;
    submesh.baseVertex = 0;
    submesh.indexOffset = 0;
    submesh.indexCount = indexCount;
    submesh.materialIndex = 0;
    subMeshes.push_back(submesh);
}

return true;
```

}

bool Mesh::LoadFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
MeshData data;
data.vertices = vertices;
data.indices = indices;
return LoadFromData(data);
}

bool Mesh::LoadOBJ(const std::string& path) {
std::ifstream file(path);
if (!file.is_open()) {
Logger::Error(“Failed to open OBJ file: “ + path);
return false;
}

```
using namespace ECS::Components;

std::vector<Vec3> positions;
std::vector<Vec3> normals;
std::vector<Vec3> texCoords;

struct VertexKey {
    int posIdx, texIdx, normIdx;
    bool operator==(const VertexKey& other) const {
        return posIdx == other.posIdx && texIdx == other.texIdx && normIdx == other.normIdx;
    }
};

struct VertexKeyHash {
    size_t operator()(const VertexKey& k) const {
        return std::hash<int>()(k.posIdx) ^ (std::hash<int>()(k.texIdx) << 1) ^ (std::hash<int>()(k.normIdx) << 2);
    }
};

std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexCache;
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

std::string line;
while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;
    
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;
    
    if (prefix == "v") {
        Vec3 pos;
        iss >> pos.x >> pos.y >> pos.z;
        positions.push_back(pos);
    }
    else if (prefix == "vn") {
        Vec3 norm;
        iss >> norm.x >> norm.y >> norm.z;
        normals.push_back(norm);
    }
    else if (prefix == "vt") {
        Vec3 tex;
        iss >> tex.x >> tex.y;
        if (!(iss >> tex.z)) tex.z = 0.0f;
        texCoords.push_back(tex);
    }
    else if (prefix == "f") {
        std::string vertexStr;
        std::vector<uint32_t> faceIndices;
        
        while (iss >> vertexStr) {
            VertexKey key = {0, 0, 0};
            
            size_t slash1 = vertexStr.find('/');
            if (slash1 != std::string::npos) {
                key.posIdx = std::stoi(vertexStr.substr(0, slash1));
                size_t slash2 = vertexStr.find('/', slash1 + 1);
                if (slash2 != std::string::npos) {
                    if (slash2 > slash1 + 1) {
                        key.texIdx = std::stoi(vertexStr.substr(slash1 + 1, slash2 - slash1 - 1));
                    }
                    key.normIdx = std::stoi(vertexStr.substr(slash2 + 1));
                } else if (slash1 + 1 < vertexStr.length()) {
                    key.texIdx = std::stoi(vertexStr.substr(slash1 + 1));
                }
            } else {
                key.posIdx = std::stoi(vertexStr);
            }
            
            auto it = vertexCache.find(key);
            if (it != vertexCache.end()) {
                faceIndices.push_back(it->second);
            } else {
                Vertex v;
                v.position = positions[key.posIdx - 1];
                v.normal = key.normIdx > 0 ? normals[key.normIdx - 1] : Vec3::Up();
                if (key.texIdx > 0 && key.texIdx <= (int)texCoords.size()) {
                    v.texCoordX = texCoords[key.texIdx - 1].x;
                    v.texCoordY = texCoords[key.texIdx - 1].y;
                } else {
                    v.texCoordX = 0.0f;
                    v.texCoordY = 0.0f;
                }
                
                uint32_t index = static_cast<uint32_t>(vertices.size());
                vertices.push_back(v);
                vertexCache[key] = index;
                faceIndices.push_back(index);
            }
        }
        
        for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
            indices.push_back(faceIndices[0]);
            indices.push_back(faceIndices[i]);
            indices.push_back(faceIndices[i + 1]);
        }
    }
}

file.close();

if (vertices.empty()) {
    Logger::Error("No vertices loaded from OBJ: " + path);
    return false;
}

return LoadFromData(vertices, indices);
```

}

bool Mesh::LoadFBX(const std::string& path) {
Logger::Warning(“FBX loading not implemented yet: “ + path);
return false;
}

bool Mesh::LoadGLTF(const std::string& path) {
Logger::Warning(“GLTF loading not implemented yet: “ + path);
return false;
}

void Mesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
Cleanup();

```
vertexCount = static_cast<uint32_t>(vertices.size());
indexCount = static_cast<uint32_t>(indices.size());

glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
if (!indices.empty()) {
    glGenBuffers(1, &ebo);
}

glBindVertexArray(vao);

glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

if (!indices.empty()) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
}

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

glEnableVertexAttribArray(3);
glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoordX));

glBindVertexArray(0);
```

}

void Mesh::CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
using namespace ECS::Components;

```
for (size_t i = 0; i < indices.size(); i += 3) {
    Vertex& v0 = vertices[indices[i]];
    Vertex& v1 = vertices[indices[i + 1]];
    Vertex& v2 = vertices[indices[i + 2]];
    
    Vec3 edge1 = v1.position - v0.position;
    Vec3 edge2 = v2.position - v0.position;
    
    float deltaU1 = v1.texCoordX - v0.texCoordX;
    float deltaV1 = v1.texCoordY - v0.texCoordY;
    float deltaU2 = v2.texCoordX - v0.texCoordX;
    float deltaV2 = v2.texCoordY - v0.texCoordY;
    
    float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);
    
    Vec3 tangent;
    tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
    tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
    tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);
    tangent = tangent.Normalized();
    
    v0.tangent += tangent;
    v1.tangent += tangent;
    v2.tangent += tangent;
}

for (auto& v : vertices) {
    v.tangent = v.tangent.Normalized();
}
```

}

void Mesh::Cleanup() {
if (vao) {
glDeleteVertexArrays(1, &vao);
vao = 0;
}
if (vbo) {
glDeleteBuffers(1, &vbo);
vbo = 0;
}
if (ebo) {
glDeleteBuffers(1, &ebo);
ebo = 0;
}
vertexCount = 0;
indexCount = 0;
subMeshes.clear();
}

void Mesh::Bind() const {
if (vao) glBindVertexArray(vao);
}

void Mesh::Unbind() const {
glBindVertexArray(0);
}

void Mesh::Draw() const {
if (!IsValid()) return;

```
Bind();
if (indexCount > 0) {
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
} else {
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
Unbind();
```

}

void Mesh::DrawSubMesh(uint32_t subMeshIndex) const {
if (!IsValid() || subMeshIndex >= subMeshes.size()) return;

```
const SubMesh& submesh = subMeshes[subMeshIndex];

Bind();
if (glDrawElementsBaseVertex) {
    glDrawElementsBaseVertex(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, 
                            (void*)(submesh.indexOffset * sizeof(uint32_t)), submesh.baseVertex);
} else {
    glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, 
                  (void*)(submesh.indexOffset * sizeof(uint32_t)));
}
Unbind();
```

}

Mesh* Mesh::CreateCube() {
using namespace ECS::Components;

```
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, Vec3::Zero(), 0.0f, 1.0f},
    
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, Vec3::Zero(), 0.0f, 1.0f},
    
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, Vec3::Zero(), 0.0f, 1.0f},
    
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, Vec3::Zero(), 0.0f, 1.0f},
    
    {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, Vec3::Zero(), 0.0f, 1.0f},
    
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, Vec3::Zero(), 0.0f, 1.0f}
};

std::vector<uint32_t> indices = {
    0,  1,  2,  2,  3,  0,
    4,  5,  6,  6,  7,  4,
    8,  9,  10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

Mesh* Mesh::CreateSphere(uint32_t segments) {
using namespace ECS::Components;

```
if (segments < 4) segments = 4;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const float PI = 3.14159265359f;

for (uint32_t lat = 0; lat <= segments; ++lat) {
    float theta = lat * PI / segments;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    
    for (uint32_t lon = 0; lon <= segments; ++lon) {
        float phi = lon * 2.0f * PI / segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        Vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
        Vec3 position = normal * 0.5f;
        
        Vertex v;
        v.position = position;
        v.normal = normal;
        v.texCoordX = (float)lon / segments;
        v.texCoordY = (float)lat / segments;
        vertices.push_back(v);
    }
}

for (uint32_t lat = 0; lat < segments; ++lat) {
    for (uint32_t lon = 0; lon < segments; ++lon) {
        uint32_t first = lat * (segments + 1) + lon;
        uint32_t second = first + segments + 1;
        
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);
        
        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
    }
}

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

Mesh* Mesh::CreateCylinder(uint32_t segments) {
using namespace ECS::Components;

```
if (segments < 3) segments = 3;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const float PI = 3.14159265359f;
const float radius = 0.5f;
const float cylinderHeight = 0.5f;
const float halfCylinder = cylinderHeight * 0.5f;

uint32_t rings = segments / 2;

for (uint32_t lat = 0; lat <= rings; ++lat) {
    float theta = lat * (PI * 0.5f) / rings;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    
    for (uint32_t lon = 0; lon <= segments; ++lon) {
        float phi = lon * 2.0f * PI / segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        Vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
        Vec3 position = normal * radius;
        position.y += halfCylinder;
        
        Vertex v;
        v.position = position;
        v.normal = normal;
        v.texCoordX = (float)lon / segments;
        v.texCoordY = (float)lat / rings * 0.5f + 0.5f;
        vertices.push_back(v);
    }
}

uint32_t topOffset = static_cast<uint32_t>(vertices.size());

for (uint32_t lat = 0; lat <= rings; ++lat) {
    float theta = lat * (PI * 0.5f) / rings;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    
    for (uint32_t lon = 0; lon <= segments; ++lon) {
        float phi = lon * 2.0f * PI / segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        Vec3 normal(cosPhi * sinTheta, -cosTheta, sinPhi * sinTheta);
        Vec3 position = normal * radius;
        position.y -= halfCylinder;
        
        Vertex v;
        v.position = position;
        v.normal = normal;
        v.texCoordX = (float)lon / segments;
        v.texCoordY = (float)lat / rings * 0.5f;
        vertices.push_back(v);
    }
}

for (uint32_t lat = 0; lat < rings; ++lat) {
    for (uint32_t lon = 0; lon < segments; ++lon) {
        uint32_t first = lat * (segments + 1) + lon;
        uint32_t second = first + segments + 1;
        
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);
        
        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
    }
}

for (uint32_t lat = 0; lat < rings; ++lat) {
    for (uint32_t lon = 0; lon < segments; ++lon) {
        uint32_t first = topOffset + lat * (segments + 1) + lon;
        uint32_t second = first + segments + 1;
        
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);
        
        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
    }
}

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

Mesh* Mesh::CreatePlane(uint32_t subdivisions) {
using namespace ECS::Components;

```
if (subdivisions < 1) subdivisions = 1;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

uint32_t verticesPerSide = subdivisions + 1;

for (uint32_t z = 0; z < verticesPerSide; ++z) {
    for (uint32_t x = 0; x < verticesPerSide; ++x) {
        float xPos = (float)x / subdivisions - 0.5f;
        float zPos = (float)z / subdivisions - 0.5f;
        
        Vertex v;
        v.position = Vec3(xPos, 0.0f, zPos);
        v.normal = Vec3::Up();
        v.texCoordX = (float)x / subdivisions;
        v.texCoordY = (float)z / subdivisions;
        vertices.push_back(v);
    }
}

for (uint32_t z = 0; z < subdivisions; ++z) {
    for (uint32_t x = 0; x < subdivisions; ++x) {
        uint32_t topLeft = z * verticesPerSide + x;
        uint32_t topRight = topLeft + 1;
        uint32_t bottomLeft = (z + 1) * verticesPerSide + x;
        uint32_t bottomRight = bottomLeft + 1;
        
        indices.push_back(topLeft);
        indices.push_back(bottomLeft);
        indices.push_back(topRight);
        
        indices.push_back(topRight);
        indices.push_back(bottomLeft);
        indices.push_back(bottomRight);
    }
}

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

Mesh* Mesh::CreateQuad() {
using namespace ECS::Components;

```
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, Vec3::Zero(), 0.0f, 0.0f},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, Vec3::Zero(), 1.0f, 0.0f},
    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, Vec3::Zero(), 1.0f, 1.0f},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, Vec3::Zero(), 0.0f, 1.0f}
};

std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

namespace MeshLibrary {
static std::unordered_map<std::string, std::unique_ptr<Mesh>> g_meshes;

```
Mesh* Load(const std::string& name, const std::string& path) {
    auto it = g_meshes.find(name);
    if (it != g_meshes.end()) return it->second.get();
    
    auto mesh = std::make_unique<Mesh>();
    if (mesh->LoadFromFile(path)) {
        Mesh* ptr = mesh.get();
        g_meshes[name] = std::move(mesh);
        Logger::Info("Loaded mesh: " + name + " from " + path);
        return ptr;
    }
    
    Logger::Error("Failed to load mesh: " + name + " from " + path);
    return nullptr;
}

Mesh* Get(const std::string& name) {
    auto it = g_meshes.find(name);
    return (it != g_meshes.end()) ? it->second.get() : nullptr;
}

Mesh* GetOrCreate(const std::string& name, const std::string& path) {
    Mesh* existing = Get(name);
    if (existing) return existing;
    return Load(name, path);
}

void Unload(const std::string& name) {
    auto it = g_meshes.find(name);
    if (it != g_meshes.end()) {
        Logger::Info("Unloaded mesh: " + name);
        g_meshes.erase(it);
    }
}

void Clear() {
    size_t count = g_meshes.size();
    g_meshes.clear();
    Logger::Info("Cleared " + std::to_string(count) + " meshes from library");
}

size_t GetLoadedCount() {
    return g_meshes.size();
}
```

}

MeshBuilder& MeshBuilder::AddVertex(const Vertex& v) {
data.vertices.push_back(v);
return *this;
}

MeshBuilder& MeshBuilder::AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
data.indices.push_back(i0);
data.indices.push_back(i1);
data.indices.push_back(i2);
return *this;
}

MeshBuilder& MeshBuilder::AddQuad(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) {
AddTriangle(i0, i1, i2);
AddTriangle(i2, i3, i0);
return *this;
}

MeshBuilder& MeshBuilder::RecalculateNormals() {
using namespace ECS::Components;

```
for (auto& v : data.vertices) {
    v.normal = Vec3::Zero();
}

for (size_t i = 0; i < data.indices.size(); i += 3) {
    uint32_t i0 = data.indices[i];
    uint32_t i1 = data.indices[i + 1];
    uint32_t i2 = data.indices[i + 2];
    
    Vec3 v0 = data.vertices[i0].position;
    Vec3 v1 = data.vertices[i1].position;
    Vec3 v2 = data.vertices[i2].position;
    
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    Vec3 normal = edge1.Cross(edge2);
    
    data.vertices[i0].normal += normal;
    data.vertices[i1].normal += normal;
    data.vertices[i2].normal += normal;
}

for (auto& v : data.vertices) {
    v.normal = v.normal.Normalized();
}

return *this;
```

}

MeshBuilder& MeshBuilder::RecalculateTangents() {
using namespace ECS::Components;

```
for (auto& v : data.vertices) {
    v.tangent = Vec3::Zero();
}

for (size_t i = 0; i < data.indices.size(); i += 3) {
    Vertex& v0 = data.vertices[data.indices[i]];
    Vertex& v1 = data.vertices[data.indices[i + 1]];
    Vertex& v2 = data.vertices[data.indices[i + 2]];
    
    Vec3 edge1 = v1.position - v0.position;
    Vec3 edge2 = v2.position - v0.position;
    
    float deltaU1 = v1.texCoordX - v0.texCoordX;
    float deltaV1 = v1.texCoordY - v0.texCoordY;
    float deltaU2 = v2.texCoordX - v0.texCoordX;
    float deltaV2 = v2.texCoordY - v0.texCoordY;
    
    float denom = deltaU1 * deltaV2 - deltaU2 * deltaV1;
    float f = (std::abs(denom) > 0.0001f) ? (1.0f / denom) : 0.0f;
    
    Vec3 tangent;
    tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
    tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
    tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);
    
    v0.tangent += tangent;
    v1.tangent += tangent;
    v2.tangent += tangent;
}

for (auto& v : data.vertices) {
    if (v.tangent.LengthSquared() > 0.0001f) {
        v.tangent = v.tangent.Normalized();
    }
}

return *this;
```

}

MeshBuilder& MeshBuilder::Transform(const ECS::Components::Transform& transform) {
using namespace ECS::Components;

```
for (auto& v : data.vertices) {
    Vec3 scaled = Vec3(v.position.x * transform.scale.x,
                      v.position.y * transform.scale.y,
                      v.position.z * transform.scale.z);
    Vec3 rotated = transform.rotation * scaled;
    v.position = rotated + transform.position;
    
    v.normal = transform.rotation * v.normal;
    v.tangent = transform.rotation * v.tangent;
}

return *this;
```

}

Mesh* MeshBuilder::Build() {
if (data.vertices.empty()) {
Logger::Error(“MeshBuilder: Cannot build mesh with no vertices”);
return nullptr;
}

```
auto mesh = new Mesh();
if (mesh->LoadFromData(data)) {
    return mesh;
}

delete mesh;
return nullptr;
```

}

}
5f;
const float height = 1.0f;
const float halfHeight = height * 0.5f;

```
Vec3 topCenter(0.0f, halfHeight, 0.0f);
Vec3 bottomCenter(0.0f, -halfHeight, 0.0f);

for (uint32_t i = 0; i <= segments; ++i) {
    float angle = (float)i * 2.0f * PI / segments;
    float x = std::cos(angle) * radius;
    float z = std::sin(angle) * radius;
    
    Vec3 normal(x, 0.0f, z);
    normal = normal.Normalized();
    
    Vertex vTop, vBottom;
    vTop.position = Vec3(x, halfHeight, z);
    vTop.normal = normal;
    vTop.texCoordX = (float)i / segments;
    vTop.texCoordY = 1.0f;
    
    vBottom.position = Vec3(x, -halfHeight, z);
    vBottom.normal = normal;
    vBottom.texCoordX = (float)i / segments;
    vBottom.texCoordY = 0.0f;
    
    vertices.push_back(vBottom);
    vertices.push_back(vTop);
}

for (uint32_t i = 0; i < segments; ++i) {
    uint32_t base = i * 2;
    indices.push_back(base);
    indices.push_back(base + 2);
    indices.push_back(base + 1);
    
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

uint32_t topCenterIdx = static_cast<uint32_t>(vertices.size());
Vertex vTopCenter;
vTopCenter.position = topCenter;
vTopCenter.normal = Vec3::Up();
vTopCenter.texCoordX = 0.5f;
vTopCenter.texCoordY = 0.5f;
vertices.push_back(vTopCenter);

uint32_t bottomCenterIdx = static_cast<uint32_t>(vertices.size());
Vertex vBottomCenter;
vBottomCenter.position = bottomCenter;
vBottomCenter.normal = Vec3::Down();
vBottomCenter.texCoordX = 0.5f;
vBottomCenter.texCoordY = 0.5f;
vertices.push_back(vBottomCenter);

for (uint32_t i = 0; i < segments; ++i) {
    uint32_t next = (i + 1) % segments;
    
    indices.push_back(topCenterIdx);
    indices.push_back(i * 2 + 1);
    indices.push_back(next * 2 + 1);
    
    indices.push_back(bottomCenterIdx);
    indices.push_back(next * 2);
    indices.push_back(i * 2);
}

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

Mesh* Mesh::CreateCapsule(uint32_t segments) {
using namespace ECS::Components;

```
if (segments < 4) segments = 4;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const float PI = 3.14159265359f;
const float radius = 0.
//will end soon, sorry :c (done)
Mesh* Mesh::CreateCapsule(uint32_t segments) {
using namespace ECS::Components;

```
if (segments < 4) segments = 4;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

const float PI = 3.14159265359f;
const float radius = 0.5f;
const float cylinderHeight = 1.0f;
const float halfCylinder = cylinderHeight * 0.5f;

uint32_t rings = segments / 2;

for (uint32_t lat = 0; lat <= rings; ++lat) {
    float theta = lat * (PI * 0.5f) / rings;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    
    for (uint32_t lon = 0; lon <= segments; ++lon) {
        float phi = lon * 2.0f * PI / segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        Vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
        Vec3 position = normal * radius;
        position.y += halfCylinder;
        
        Vertex v;
        v.position = position;
        v.normal = normal;
        v.texCoordX = (float)lon / segments;
        v.texCoordY = 1.0f - (float)lat / rings * 0.5f;
        vertices.push_back(v);
    }
}

uint32_t topHemisphereEnd = static_cast<uint32_t>(vertices.size());

for (uint32_t i = 0; i <= segments; ++i) {
    float angle = (float)i * 2.0f * PI / segments;
    float x = std::cos(angle) * radius;
    float z = std::sin(angle) * radius;
    
    Vec3 normal(x, 0.0f, z);
    normal = normal.Normalized();
    
    Vertex vTop, vBottom;
    vTop.position = Vec3(x, halfCylinder, z);
    vTop.normal = normal;
    vTop.texCoordX = (float)i / segments;
    vTop.texCoordY = 0.5f;
    
    vBottom.position = Vec3(x, -halfCylinder, z);
    vBottom.normal = normal;
    vBottom.texCoordX = (float)i / segments;
    vBottom.texCoordY = 0.5f;
    
    vertices.push_back(vTop);
    vertices.push_back(vBottom);
}

uint32_t cylinderEnd = static_cast<uint32_t>(vertices.size());

for (uint32_t lat = 0; lat <= rings; ++lat) {
    float theta = lat * (PI * 0.5f) / rings;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    
    for (uint32_t lon = 0; lon <= segments; ++lon) {
        float phi = lon * 2.0f * PI / segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        Vec3 normal(cosPhi * sinTheta, -cosTheta, sinPhi * sinTheta);
        Vec3 position = normal * radius;
        position.y -= halfCylinder;
        
        Vertex v;
        v.position = position;
        v.normal = normal;
        v.texCoordX = (float)lon / segments;
        v.texCoordY = 0.5f - (float)lat / rings * 0.5f;
        vertices.push_back(v);
    }
}

for (uint32_t lat = 0; lat < rings; ++lat) {
    for (uint32_t lon = 0; lon < segments; ++lon) {
        uint32_t first = lat * (segments + 1) + lon;
        uint32_t second = first + segments + 1;
        
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);
        
        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
    }
}

uint32_t cylinderStart = topHemisphereEnd;
for (uint32_t i = 0; i < segments; ++i) {
    uint32_t base = cylinderStart + i * 2;
    indices.push_back(base);
    indices.push_back(base + 2);
    indices.push_back(base + 1);
    
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

uint32_t bottomStart = cylinderEnd;
for (uint32_t lat = 0; lat < rings; ++lat) {
    for (uint32_t lon = 0; lon < segments; ++lon) {
        uint32_t first = bottomStart + lat * (segments + 1) + lon;
        uint32_t second = first + segments + 1;
        
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);
        
        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
    }
}

Mesh* mesh = new Mesh();
mesh->LoadFromData(vertices, indices);
return mesh;
```

}

}
