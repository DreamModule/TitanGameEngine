#pragma once

#include <string>
#include <cstdint>
#include “Transform.hpp”

namespace Titan {
namespace ECS {
namespace Components {

struct Color {
float r = 1.0f;
float g = 1.0f;
float b = 1.0f;
float a = 1.0f;

```
Color() = default;
Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

static Color White() { return Color(1, 1, 1, 1); }
static Color Black() { return Color(0, 0, 0, 1); }
static Color Red() { return Color(1, 0, 0, 1); }
static Color Green() { return Color(0, 1, 0, 1); }
static Color Blue() { return Color(0, 0, 1, 1); }
static Color Yellow() { return Color(1, 1, 0, 1); }
static Color Cyan() { return Color(0, 1, 1, 1); }
static Color Magenta() { return Color(1, 0, 1, 1); }
static Color Transparent() { return Color(0, 0, 0, 0); }
```

};

struct MeshRenderer {
uint32_t meshId = 0;
uint32_t materialId = 0;

```
std::string meshPath;
std::string texturePath;

Color tint = Color::White();

bool castShadows = true;
bool receiveShadows = true;
bool visible = true;

int renderLayer = 0;

MeshRenderer() = default;

MeshRenderer(const std::string& meshPath) 
    : meshPath(meshPath) {}

MeshRenderer(const std::string& meshPath, const std::string& texturePath)
    : meshPath(meshPath), texturePath(texturePath) {}
```

};

}
}
}
