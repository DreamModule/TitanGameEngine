#pragma once

#include “../ECS/Components/Transform.hpp”
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace Titan::Graphics {

struct Vertex {
ECS::Components::Vec3 position;
ECS::Components::Vec3 normal;
ECS::Components::Vec3 tangent;
float texCoordX;
float texCoordY;

```
Vertex() = default;
Vertex(const ECS::Components::Vec3& pos, const ECS::Components::Vec3& norm, float u, float v)
    : position(pos), normal(norm), tangent(ECS::Components::Vec3::Zero()), texCoordX(u), texCoordY(v) {}
```

};

struct SubMesh {
uint32_t baseVertex;
uint32_t indexOffset;
uint32_t indexCount;
uint32_t materialIndex;
};

struct MeshData {
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
std::vector<SubMesh> subMeshes;
};

class Mesh {
public:
Mesh();
~Mesh();

```
Mesh(const Mesh&) = delete;
Mesh& operator=(const Mesh&) = delete;
Mesh(Mesh&& other) noexcept;
Mesh& operator=(Mesh&& other) noexcept;

bool LoadFromFile(const std::string& path);
bool LoadFromData(const MeshData& data);
bool LoadFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

void Bind() const;
void Unbind() const;
void Draw() const;
void DrawSubMesh(uint32_t subMeshIndex) const;

uint32_t GetVertexCount() const { return vertexCount; }
uint32_t GetIndexCount() const { return indexCount; }
uint32_t GetSubMeshCount() const { return static_cast<uint32_t>(subMeshes.size()); }
bool IsValid() const { return vao != 0; }

const std::vector<SubMesh>& GetSubMeshes() const { return subMeshes; }

static Mesh* CreateCube();
static Mesh* CreateSphere(uint32_t segments = 32);
static Mesh* CreateCylinder(uint32_t segments = 32);
static Mesh* CreateCapsule(uint32_t segments = 16);
static Mesh* CreatePlane(uint32_t subdivisions = 1);
static Mesh* CreateQuad();
```

private:
uint32_t vao;
uint32_t vbo;
uint32_t ebo;
uint32_t vertexCount;
uint32_t indexCount;

```
std::vector<SubMesh> subMeshes;

void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
void Cleanup();
void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

bool LoadOBJ(const std::string& path);
bool LoadFBX(const std::string& path);
bool LoadGLTF(const std::string& path);
```

};

namespace MeshLibrary {
Mesh* Load(const std::string& name, const std::string& path);
Mesh* Get(const std::string& name);
Mesh* GetOrCreate(const std::string& name, const std::string& path);
void Unload(const std::string& name);
void Clear();
size_t GetLoadedCount();
}

struct MeshBuilder {
MeshData data;

```
MeshBuilder& AddVertex(const Vertex& v);
MeshBuilder& AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
MeshBuilder& AddQuad(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3);
MeshBuilder& RecalculateNormals();
MeshBuilder& RecalculateTangents();
MeshBuilder& Transform(const ECS::Components::Transform& transform);

Mesh* Build();
```

};

}
