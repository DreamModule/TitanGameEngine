#pragma once

#include “Shader.hpp”
#include “Texture.hpp”
#include “../ECS/Components/MeshRenderer.hpp”
#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace Titan::Graphics {

using namespace ECS::Components;

enum class RenderQueue {
Background = 1000,
Geometry = 2000,
AlphaTest = 2450,
Transparent = 3000,
Overlay = 4000
};

enum class CullMode {
Off,
Front,
Back
};

enum class BlendMode {
Opaque,
AlphaBlend,
Additive,
Multiply
};

enum class DepthTest {
Less,
LessEqual,
Greater,
GreaterEqual,
Equal,
NotEqual,
Always,
Never
};

struct MaterialProperties {
Color albedo = Color::White();
float metallic = 0.0f;
float roughness = 0.5f;
float ao = 1.0f;
Vec3 emission = Vec3::Zero();
float emissionIntensity = 0.0f;

```
Texture* albedoMap = nullptr;
Texture* normalMap = nullptr;
Texture* metallicMap = nullptr;
Texture* roughnessMap = nullptr;
Texture* aoMap = nullptr;
Texture* emissionMap = nullptr;

bool useAlbedoMap = false;
bool useNormalMap = false;
bool useMetallicMap = false;
bool useRoughnessMap = false;
bool useAOMap = false;
bool useEmissionMap = false;

float alphaCutoff = 0.5f;
Vec2 textureScale = {1.0f, 1.0f};
Vec2 textureOffset = {0.0f, 0.0f};
```

};

class Material {
public:
Material();
explicit Material(Shader* shader);
~Material() = default;

```
Material(const Material&) = delete;
Material& operator=(const Material&) = delete;
Material(Material&&) noexcept = default;
Material& operator=(Material&&) noexcept = default;

void SetShader(Shader* shader);
Shader* GetShader() const { return shader; }

void SetTexture(const std::string& name, Texture* texture);
Texture* GetTexture(const std::string& name) const;

void SetInt(const std::string& name, int value);
void SetFloat(const std::string& name, float value);
void SetVec2(const std::string& name, const Vec2& value);
void SetVec3(const std::string& name, const Vec3& value);
void SetVec4(const std::string& name, const Vec4& value);
void SetColor(const std::string& name, const Color& value);
void SetMat4(const std::string& name, const float* matrix);

int GetInt(const std::string& name) const;
float GetFloat(const std::string& name) const;

void SetRenderQueue(RenderQueue queue) { renderQueue = static_cast<int>(queue); }
void SetRenderQueue(int queue) { renderQueue = queue; }
int GetRenderQueue() const { return renderQueue; }

void SetCullMode(CullMode mode) { cullMode = mode; }
CullMode GetCullMode() const { return cullMode; }

void SetBlendMode(BlendMode mode) { blendMode = mode; }
BlendMode GetBlendMode() const { return blendMode; }

void SetDepthTest(DepthTest test) { depthTest = test; }
DepthTest GetDepthTest() const { return depthTest; }

void SetDepthWrite(bool enabled) { depthWrite = enabled; }
bool GetDepthWrite() const { return depthWrite; }

void SetWireframe(bool enabled) { wireframe = enabled; }
bool IsWireframe() const { return wireframe; }

MaterialProperties& GetProperties() { return properties; }
const MaterialProperties& GetProperties() const { return properties; }

void Bind();
void Unbind();

void UploadUniforms();

uint64_t GetSortKey() const;

bool IsTransparent() const {
    return blendMode != BlendMode::Opaque;
}
```

private:
Shader* shader;
MaterialProperties properties;

```
std::unordered_map<std::string, Texture*> textureSlots;
std::unordered_map<std::string, int> intUniforms;
std::unordered_map<std::string, float> floatUniforms;
std::unordered_map<std::string, Vec2> vec2Uniforms;
std::unordered_map<std::string, Vec3> vec3Uniforms;
std::unordered_map<std::string, Vec4> vec4Uniforms;

int renderQueue;
CullMode cullMode;
BlendMode blendMode;
DepthTest depthTest;
bool depthWrite;
bool wireframe;

void ApplyRenderState();
```

};

namespace MaterialLibrary {
Material* Create(const std::string& name, Shader* shader);
Material* Get(const std::string& name);
Material* GetOrCreate(const std::string& name, Shader* shader);
void Unload(const std::string& name);
void Clear();
size_t GetLoadedCount();
}

struct MaterialPresets {
static Material* CreateUnlit(const Color& color = Color::White());
static Material* CreateLit(const Color& color = Color::White());
static Material* CreatePBR(const Color& albedo = Color::White(), float metallic = 0.0f, float roughness = 0.5f);
static Material* CreateTransparent(const Color& color = Color::White(), float alpha = 0.5f);
static Material* CreateWireframe(const Color& color = Color::White());
};

}
