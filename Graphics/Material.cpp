#include “Material.hpp”
#include “../Core/Logger.hpp”
#include <windows.h>
#include <gl/GL.h>

#ifndef GL_CULL_FACE
#define GL_CULL_FACE 0x0B44
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE 0x0001
#define GL_DEPTH_TEST 0x0B71
#define GL_LEQUAL 0x0203
#define GL_GEQUAL 0x0206
#define GL_EQUAL 0x0202
#define GL_NOTEQUAL 0x0205
#define GL_ALWAYS 0x0207
#define GL_NEVER 0x0200
#define GL_GREATER 0x0204
#define GL_LESS 0x0201
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#endif

typedef void (APIENTRY *PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void (APIENTRY *PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void (APIENTRY *PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void (APIENTRY *PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);

namespace {
PFNGLBLENDFUNCPROC glBlendFunc = nullptr;
PFNGLDEPTHFUNCPROC glDepthFunc = nullptr;
PFNGLDEPTHMASKPROC glDepthMask = nullptr;
PFNGLPOLYGONMODEPROC glPolygonMode = nullptr;

```
bool g_glFunctionsLoaded = false;

void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;
    
    HMODULE opengl = LoadLibraryA("opengl32.dll");
    if (opengl) {
        glBlendFunc = (PFNGLBLENDFUNCPROC)GetProcAddress(opengl, "glBlendFunc");
        glDepthFunc = (PFNGLDEPTHFUNCPROC)GetProcAddress(opengl, "glDepthFunc");
        glDepthMask = (PFNGLDEPTHMASKPROC)GetProcAddress(opengl, "glDepthMask");
        glPolygonMode = (PFNGLPOLYGONMODEPROC)GetProcAddress(opengl, "glPolygonMode");
    }
    
    g_glFunctionsLoaded = true;
}
```

}

namespace Titan::Graphics {

Material::Material()
: shader(nullptr)
, renderQueue(static_cast<int>(RenderQueue::Geometry))
, cullMode(CullMode::Back)
, blendMode(BlendMode::Opaque)
, depthTest(DepthTest::Less)
, depthWrite(true)
, wireframe(false) {
LoadGLFunctions();
}

Material::Material(Shader* shader)
: shader(shader)
, renderQueue(static_cast<int>(RenderQueue::Geometry))
, cullMode(CullMode::Back)
, blendMode(BlendMode::Opaque)
, depthTest(DepthTest::Less)
, depthWrite(true)
, wireframe(false) {
LoadGLFunctions();
}

void Material::SetShader(Shader* newShader) {
shader = newShader;
}

void Material::SetTexture(const std::string& name, Texture* texture) {
textureSlots[name] = texture;
}

Texture* Material::GetTexture(const std::string& name) const {
auto it = textureSlots.find(name);
return (it != textureSlots.end()) ? it->second : nullptr;
}

void Material::SetInt(const std::string& name, int value) {
intUniforms[name] = value;
}

void Material::SetFloat(const std::string& name, float value) {
floatUniforms[name] = value;
}

void Material::SetVec2(const std::string& name, const Vec2& value) {
vec2Uniforms[name] = value;
}

void Material::SetVec3(const std::string& name, const Vec3& value) {
vec3Uniforms[name] = value;
}

void Material::SetVec4(const std::string& name, const Vec4& value) {
vec4Uniforms[name] = value;
}

void Material::SetColor(const std::string& name, const Color& value) {
Vec4 colorVec(value.r, value.g, value.b, value.a);
vec4Uniforms[name] = colorVec;
}

void Material::SetMat4(const std::string& name, const float* matrix) {
if (shader) {
shader->SetMat4(name, matrix);
}
}

int Material::GetInt(const std::string& name) const {
auto it = intUniforms.find(name);
return (it != intUniforms.end()) ? it->second : 0;
}

float Material::GetFloat(const std::string& name) const {
auto it = floatUniforms.find(name);
return (it != floatUniforms.end()) ? it->second : 0.0f;
}

void Material::ApplyRenderState() {
switch (cullMode) {
case CullMode::Off:
glDisable(GL_CULL_FACE);
break;
case CullMode::Front:
glEnable(GL_CULL_FACE);
glCullFace(GL_FRONT);
break;
case CullMode::Back:
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
break;
}

```
if (blendMode != BlendMode::Opaque) {
    glEnable(GL_BLEND);
    if (glBlendFunc) {
        switch (blendMode) {
            case BlendMode::AlphaBlend:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case BlendMode::Additive:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case BlendMode::Multiply:
                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                break;
            default:
                break;
        }
    }
} else {
    glDisable(GL_BLEND);
}

if (glDepthFunc) {
    switch (depthTest) {
        case DepthTest::Less: glDepthFunc(GL_LESS); break;
        case DepthTest::LessEqual: glDepthFunc(GL_LEQUAL); break;
        case DepthTest::Greater: glDepthFunc(GL_GREATER); break;
        case DepthTest::GreaterEqual: glDepthFunc(GL_GEQUAL); break;
        case DepthTest::Equal: glDepthFunc(GL_EQUAL); break;
        case DepthTest::NotEqual: glDepthFunc(GL_NOTEQUAL); break;
        case DepthTest::Always: glDepthFunc(GL_ALWAYS); break;
        case DepthTest::Never: glDepthFunc(GL_NEVER); break;
    }
}

if (glDepthMask) {
    glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
}

if (glPolygonMode) {
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}
```

}

void Material::Bind() {
if (!shader) return;

```
shader->Bind();
ApplyRenderState();

uint32_t textureUnit = 0;
for (auto& pair : textureSlots) {
    if (pair.second) {
        pair.second->Bind(textureUnit);
        shader->SetInt(pair.first, textureUnit);
        textureUnit++;
    }
}

UploadUniforms();
```

}

void Material::Unbind() {
if (shader) {
shader->Unbind();
}

```
glDisable(GL_BLEND);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

if (glDepthFunc) glDepthFunc(GL_LESS);
if (glDepthMask) glDepthMask(GL_TRUE);
if (glPolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
```

}

void Material::UploadUniforms() {
if (!shader) return;

```
for (auto& pair : intUniforms) {
    shader->SetInt(pair.first, pair.second);
}

for (auto& pair : floatUniforms) {
    shader->SetFloat(pair.first, pair.second);
}

for (auto& pair : vec2Uniforms) {
    shader->SetVec2(pair.first, pair.second.x, pair.second.y);
}

for (auto& pair : vec3Uniforms) {
    shader->SetVec3(pair.first, pair.second.x, pair.second.y, pair.second.z);
}

for (auto& pair : vec4Uniforms) {
    shader->SetVec4(pair.first, pair.second.x, pair.second.y, pair.second.z, pair.second.w);
}

auto& props = properties;
shader->SetVec4("uAlbedo", props.albedo.r, props.albedo.g, props.albedo.b, props.albedo.a);
shader->SetFloat("uMetallic", props.metallic);
shader->SetFloat("uRoughness", props.roughness);
shader->SetFloat("uAO", props.ao);
shader->SetVec3("uEmission", props.emission.x, props.emission.y, props.emission.z);
shader->SetFloat("uEmissionIntensity", props.emissionIntensity);
shader->SetBool("uUseAlbedoMap", props.useAlbedoMap);
shader->SetBool("uUseNormalMap", props.useNormalMap);
shader->SetBool("uUseMetallicMap", props.useMetallicMap);
shader->SetBool("uUseRoughnessMap", props.useRoughnessMap);
shader->SetBool("uUseAOMap", props.useAOMap);
shader->SetBool("uUseEmissionMap", props.useEmissionMap);
shader->SetFloat("uAlphaCutoff", props.alphaCutoff);
shader->SetVec2("uTextureScale", props.textureScale.x, props.textureScale.y);
shader->SetVec2("uTextureOffset", props.textureOffset.x, props.textureOffset.y);
```

}

uint64_t Material::GetSortKey() const {
uint64_t key = 0;

```
key |= (static_cast<uint64_t>(renderQueue) & 0xFFFF) << 48;

if (shader) {
    key |= (static_cast<uint64_t>(shader->GetProgramId()) & 0xFFFF) << 32;
}

uint32_t textureHash = 0;
for (auto& pair : textureSlots) {
    if (pair.second) {
        textureHash ^= pair.second->GetTextureId();
    }
}
key |= textureHash;

return key;
```

}

namespace MaterialLibrary {
static std::unordered_map<std::string, std::unique_ptr<Material>> g_materials;

```
Material* Create(const std::string& name, Shader* shader) {
    auto it = g_materials.find(name);
    if (it != g_materials.end()) return it->second.get();

    auto material = std::make_unique<Material>(shader);
    Material* ptr = material.get();
    g_materials[name] = std::move(material);
    Logger::Info("Created material: " + name);
    return ptr;
}

Material* Get(const std::string& name) {
    auto it = g_materials.find(name);
    return (it != g_materials.end()) ? it->second.get() : nullptr;
}

Material* GetOrCreate(const std::string& name, Shader* shader) {
    Material* existing = Get(name);
    if (existing) return existing;
    return Create(name, shader);
}

void Unload(const std::string& name) {
    auto it = g_materials.find(name);
    if (it != g_materials.end()) {
        Logger::Info("Unloaded material: " + name);
        g_materials.erase(it);
    }
}

void Clear() {
    size_t count = g_materials.size();
    g_materials.clear();
    Logger::Info("Cleared " + std::to_string(count) + " materials from library");
}

size_t GetLoadedCount() {
    return g_materials.size();
}
```

}

Material* MaterialPresets::CreateUnlit(const Color& color) {
Shader* shader = ShaderPresets::CreateUnlit();
if (!shader) return nullptr;

```
auto material = new Material(shader);
material->GetProperties().albedo = color;
material->SetRenderQueue(RenderQueue::Geometry);
return material;
```

}

Material* MaterialPresets::CreateLit(const Color& color) {
Shader* shader = ShaderPresets::CreateLit();
if (!shader) return nullptr;

```
auto material = new Material(shader);
material->GetProperties().albedo = color;
material->SetRenderQueue(RenderQueue::Geometry);
return material;
```

}

Material* MaterialPresets::CreatePBR(const Color& albedo, float metallic, float roughness) {
Shader* shader = ShaderPresets::CreatePBR();
if (!shader) return nullptr;

```
auto material = new Material(shader);
material->GetProperties().albedo = albedo;
material->GetProperties().metallic = metallic;
material->GetProperties().roughness = roughness;
material->SetRenderQueue(RenderQueue::Geometry);
return material;
```

}

Material* MaterialPresets::CreateTransparent(const Color& color, float alpha) {
Shader* shader = ShaderPresets::CreateUnlit();
if (!shader) return nullptr;

```
auto material = new Material(shader);
Color transparentColor = color;
transparentColor.a = alpha;
material->GetProperties().albedo = transparentColor;
material->SetBlendMode(BlendMode::AlphaBlend);
material->SetDepthWrite(false);
material->SetRenderQueue(RenderQueue::Transparent);
return material;
```

}

Material* MaterialPresets::CreateWireframe(const Color& color) {
Shader* shader = ShaderPresets::CreateUnlit();
if (!shader) return nullptr;

```
auto material = new Material(shader);
material->GetProperties().albedo = color;
material->SetWireframe(true);
material->SetCullMode(CullMode::Off);
material->SetRenderQueue(RenderQueue::Overlay);
return material;
```

}

}
