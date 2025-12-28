#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <memory>

namespace Titan::Graphics {

class Shader {
public:
Shader();
~Shader();

```
Shader(const Shader&) = delete;
Shader& operator=(const Shader&) = delete;
Shader(Shader&& other) noexcept;
Shader& operator=(Shader&& other) noexcept;

bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
bool LoadFromFiles(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath);
bool LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);
bool LoadFromSource(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc);

void Bind() const;
void Unbind() const;

void SetInt(const std::string& name, int value);
void SetFloat(const std::string& name, float value);
void SetVec2(const std::string& name, float x, float y);
void SetVec3(const std::string& name, float x, float y, float z);
void SetVec4(const std::string& name, float x, float y, float z, float w);
void SetMat3(const std::string& name, const float* matrix);
void SetMat4(const std::string& name, const float* matrix);
void SetBool(const std::string& name, bool value);

void SetIntArray(const std::string& name, int* values, uint32_t count);
void SetFloatArray(const std::string& name, float* values, uint32_t count);

uint32_t GetProgramId() const { return programId; }
bool IsValid() const { return programId != 0; }
```

private:
uint32_t programId;
mutable std::unordered_map<std::string, int32_t> uniformCache;

```
int32_t GetUniformLocation(const std::string& name) const;
uint32_t CompileShader(uint32_t type, const std::string& source);
bool LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
bool LinkProgram(uint32_t vertexShader, uint32_t geometryShader, uint32_t fragmentShader);
std::string ReadFile(const std::string& path);
void CheckCompileErrors(uint32_t shader, const std::string& type);
void CheckLinkErrors(uint32_t program);
void Cleanup();
```

};

namespace ShaderLibrary {
Shader* Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
Shader* LoadWithGeometry(const std::string& name, const std::string& vertexPath,
const std::string& geometryPath, const std::string& fragmentPath);
Shader* Get(const std::string& name);
Shader* GetOrLoad(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
void Unload(const std::string& name);
void Clear();
size_t GetLoadedCount();
}

struct ShaderPresets {
static Shader* CreateUnlit();
static Shader* CreateLit();
static Shader* CreatePBR();
static Shader* CreateSkybox();
static Shader* CreateUI();
};

}
