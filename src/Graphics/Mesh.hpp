#pragma once

#include “../ECS/Components/Transform.hpp”
#include <vector>
#include <string>
#include <cstdint>

namespace Titan {
namespace Graphics {

struct Vertex {
ECS::Components::Vec3 position;
ECS::Components::Vec3 normal;
float texCoordX;
float texCoordY;

```
Vertex() = default;

Vertex(const ECS::Components::Vec3& pos, const ECS::Components::Vec3& norm, float u, float v)
    : position(pos), normal(norm), texCoordX(u), texCoordY(v) {}
```

};

class Mesh {
public:
Mesh();
~Mesh();

```
bool LoadFromFile(const std::string& path);
bool LoadFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

void Bind() const;
void Unbind() const;
void Draw() const;

uint32_t GetVertexCount() const { return vertexCount; }
uint32_t GetIndexCount() const { return indexCount; }

bool IsValid() const { return vao != 0; }

static Mesh* CreateCube();
static Mesh* CreateSphere(uint32_t segments = 32);
static Mesh* CreatePlane();
static Mesh* CreateQuad();
```

private:
uint32_t vao;
uint32_t vbo;
uint32_t ebo;
uint32_t vertexCount;
uint32_t indexCount;

```
void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
void Cleanup();
```

};

namespace MeshLibrary {
Mesh* Load(const std::string& name, const std::string& path);
Mesh* Get(const std::string& name);
void Clear();
}

}
}
