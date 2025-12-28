#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

namespace Titan {
namespace Graphics {

class Shader {
public:
Shader();
~Shader();

```
bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
bool LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);

void Bind() const;
void Unbind() const;

void SetInt(const std::string& name, int value);
void SetFloat(const std::string& name, float value);
void SetVec2(const std::string& name, float x, float y);
void SetVec3(const std::string& name, float x, float y, float z);
void SetVec4(const std::string& name, float x, float y, float z, float w);
void SetMat4(const std::string& name, const float* matrix);

uint32_t GetProgramId() const { return programId; }
bool IsValid() const { return programId != 0; }
```

private:
uint32_t programId;
std::unordered_map<std::string, int32_t> uniformLocations;

```
int32_t GetUniformLocation(const std::string& name);
uint32_t CompileShader(uint32_t type, const std::string& source);
bool LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
```

};

namespace ShaderLibrary {
Shader* Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
Shader* Get(const std::string& name);
void Clear();
}

}
}
