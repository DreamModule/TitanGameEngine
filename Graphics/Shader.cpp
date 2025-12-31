#include “Shader.hpp”
#include “../Core/Logger.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <fstream>
#include <sstream>

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void (APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (APIENTRY *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (APIENTRY *PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY *PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY *PFNGLUNIFORMMATRIX3FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY *PFNGLUNIFORM1IVPROC)(GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY *PFNGLUNIFORM1FVPROC)(GLint location, GLsizei count, const GLfloat *value);

namespace {
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLUNIFORM1IVPROC glUniform1iv = nullptr;
PFNGLUNIFORM1FVPROC glUniform1fv = nullptr;

```
bool g_glFunctionsLoaded = false;

void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;
    
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
    glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
    
    g_glFunctionsLoaded = true;
}
```

}

namespace Titan::Graphics {

Shader::Shader() : programId(0) {
LoadGLFunctions();
}

Shader::~Shader() {
Cleanup();
}

Shader::Shader(Shader&& other) noexcept
: programId(other.programId), uniformCache(std::move(other.uniformCache)) {
other.programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
if (this != &other) {
Cleanup();
programId = other.programId;
uniformCache = std::move(other.uniformCache);
other.programId = 0;
}
return *this;
}

std::string Shader::ReadFile(const std::string& path) {
std::ifstream file(path);
if (!file.is_open()) {
Logger::Error(“Failed to open shader file: “ + path);
return “”;
}

```
std::stringstream buffer;
buffer << file.rdbuf();
return buffer.str();
```

}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
std::string vertexCode = ReadFile(vertexPath);
std::string fragmentCode = ReadFile(fragmentPath);

```
if (vertexCode.empty() || fragmentCode.empty()) {
    return false;
}

return LoadFromSource(vertexCode, fragmentCode);
```

}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath) {
std::string vertexCode = ReadFile(vertexPath);
std::string geometryCode = ReadFile(geometryPath);
std::string fragmentCode = ReadFile(fragmentPath);

```
if (vertexCode.empty() || geometryCode.empty() || fragmentCode.empty()) {
    return false;
}

return LoadFromSource(vertexCode, geometryCode, fragmentCode);
```

}

bool Shader::LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc) {
Cleanup();

```
uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
if (vertexShader == 0) return false;

uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
if (fragmentShader == 0) {
    glDeleteShader(vertexShader);
    return false;
}

bool success = LinkProgram(vertexShader, fragmentShader);

glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);

return success;
```

}

bool Shader::LoadFromSource(const std::string& vertexSrc, const std::string& geometrySrc, const std::string& fragmentSrc) {
Cleanup();

```
uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
if (vertexShader == 0) return false;

uint32_t geometryShader = CompileShader(GL_GEOMETRY_SHADER, geometrySrc);
if (geometryShader == 0) {
    glDeleteShader(vertexShader);
    return false;
}

uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
if (fragmentShader == 0) {
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    return false;
}

bool success = LinkProgram(vertexShader, geometryShader, fragmentShader);

glDeleteShader(vertexShader);
glDeleteShader(geometryShader);
glDeleteShader(fragmentShader);

return success;
```

}

uint32_t Shader::CompileShader(uint32_t type, const std::string& source) {
uint32_t shader = glCreateShader(type);
const char* src = source.c_str();
glShaderSource(shader, 1, &src, nullptr);
glCompileShader(shader);

```
std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : 
                     (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "GEOMETRY";
CheckCompileErrors(shader, typeStr);

int success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    glDeleteShader(shader);
    return 0;
}

return shader;
```

}

bool Shader::LinkProgram(uint32_t vertexShader, uint32_t fragmentShader) {
programId = glCreateProgram();
glAttachShader(programId, vertexShader);
glAttachShader(programId, fragmentShader);
glLinkProgram(programId);

```
CheckLinkErrors(programId);

int success;
glGetProgramiv(programId, GL_LINK_STATUS, &success);
if (!success) {
    glDeleteProgram(programId);
    programId = 0;
    return false;
}

uniformCache.clear();
return true;
```

}

bool Shader::LinkProgram(uint32_t vertexShader, uint32_t geometryShader, uint32_t fragmentShader) {
programId = glCreateProgram();
glAttachShader(programId, vertexShader);
glAttachShader(programId, geometryShader);
glAttachShader(programId, fragmentShader);
glLinkProgram(programId);

```
CheckLinkErrors(programId);

int success;
glGetProgramiv(programId, GL_LINK_STATUS, &success);
if (!success) {
    glDeleteProgram(programId);
    programId = 0;
    return false;
}

uniformCache.clear();
return true;
```

}

void Shader::CheckCompileErrors(uint32_t shader, const std::string& type) {
int success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
int logLength;
glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

```
    if (logLength > 0) {
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog);
        Logger::Error("Shader compilation error (" + type + "): " + std::string(infoLog));
        delete[] infoLog;
    }
}
```

}

void Shader::CheckLinkErrors(uint32_t program) {
int success;
glGetProgramiv(program, GL_LINK_STATUS, &success);
if (!success) {
int logLength;
glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

```
    if (logLength > 0) {
        char* infoLog = new char[logLength];
        glGetProgramInfoLog(program, logLength, nullptr, infoLog);
        Logger::Error("Shader linking error: " + std::string(infoLog));
        delete[] infoLog;
    }
}
```

}

void Shader::Cleanup() {
if (programId != 0) {
glDeleteProgram(programId);
programId = 0;
}
uniformCache.clear();
}

void Shader::Bind() const {
if (programId != 0) {
glUseProgram(programId);
}
}

void Shader::Unbind() const {
glUseProgram(0);
}

int32_t Shader::GetUniformLocation(const std::string& name) const {
auto it = uniformCache.find(name);
if (it != uniformCache.end()) {
return it->second;
}

```
int32_t location = glGetUniformLocation(programId, name.c_str());
uniformCache[name] = location;

if (location == -1) {
    Logger::Warning("Uniform '" + name + "' not found in shader");
}

return location;
```

}

void Shader::SetInt(const std::string& name, int value) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform1i(loc, value);
}
}

void Shader::SetFloat(const std::string& name, float value) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform1f(loc, value);
}
}

void Shader::SetVec2(const std::string& name, float x, float y) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform2f(loc, x, y);
}
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform3f(loc, x, y, z);
}
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform4f(loc, x, y, z, w);
}
}

void Shader::SetMat3(const std::string& name, const float* matrix) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniformMatrix3fv(loc, 1, GL_FALSE, matrix);
}
}

void Shader::SetMat4(const std::string& name, const float* matrix) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);
}
}

void Shader::SetBool(const std::string& name, bool value) {
SetInt(name, value ? 1 : 0);
}

void Shader::SetIntArray(const std::string& name, int* values, uint32_t count) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform1iv(loc, count, values);
}
}

void Shader::SetFloatArray(const std::string& name, float* values, uint32_t count) {
int32_t loc = GetUniformLocation(name);
if (loc != -1) {
glUniform1fv(loc, count, values);
}
}

namespace ShaderLibrary {
static std::unordered_map<std::string, std::unique_ptr<Shader>> g_shaders;

```
Shader* Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto it = g_shaders.find(name);
    if (it != g_shaders.end()) return it->second.get();
    
    auto shader = std::make_unique<Shader>();
    if (shader->LoadFromFiles(vertexPath, fragmentPath)) {
        Shader* ptr = shader.get();
        g_shaders[name] = std::move(shader);
        Logger::Info("Loaded shader: " + name);
        return ptr;
    }
    
    Logger::Error("Failed to load shader: " + name);
    return nullptr;
}

Shader* LoadWithGeometry(const std::string& name, const std::string& vertexPath, 
                        const std::string& geometryPath, const std::string& fragmentPath) {
    auto it = g_shaders.find(name);
    if (it != g_shaders.end()) return it->second.get();
    
    auto shader = std::make_unique<Shader>();
    if (shader->LoadFromFiles(vertexPath, geometryPath, fragmentPath)) {
        Shader* ptr = shader.get();
        g_shaders[name] = std::move(shader);
        Logger::Info("Loaded shader with geometry: " + name);
        return ptr;
    }
    
    Logger::Error("Failed to load shader with geometry: " + name);
    return nullptr;
}

Shader* Get(const std::string& name) {
    auto it = g_shaders.find(name);
    return (it != g_shaders.end()) ? it->second.get() : nullptr;
}

Shader* GetOrLoad(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    Shader* existing = Get(name);
    if (existing) return existing;
    return Load(name, vertexPath, fragmentPath);
}

void Unload(const std::string& name) {
    auto it = g_shaders.find(name);
    if (it != g_shaders.end()) {
        Logger::Info("Unloaded shader: " + name);
        g_shaders.erase(it);
    }
}

void Clear() {
    size_t count = g_shaders.size();
    g_shaders.clear();
    Logger::Info("Cleared " + std::to_string(count) + " shaders from library");
}

size_t GetLoadedCount() {
    return g_shaders.size();
}
```

}

Shader* ShaderPresets::CreateUnlit() {
const char* vertexSrc = R”(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec2 aTexCoord;

```
    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    
    out vec2 vTexCoord;
    
    void main() {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        vTexCoord = aTexCoord;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    out vec4 FragColor;
    
    uniform vec4 uColor;
    uniform sampler2D uTexture;
    uniform bool uUseTexture;
    
    void main() {
        if (uUseTexture) {
            FragColor = texture(uTexture, vTexCoord) * uColor;
        } else {
            FragColor = uColor;
        }
    }
)";

Shader* shader = new Shader();
shader->LoadFromSource(vertexSrc, fragmentSrc);
return shader;
```

}

Shader* ShaderPresets::CreateLit() {
const char* vertexSrc = R”(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoord;

```
    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    
    out vec3 vFragPos;
    out vec3 vNormal;
    out vec2 vTexCoord;
    
    void main() {
        vec4 worldPos = uModel * vec4(aPos, 1.0);
        vFragPos = worldPos.xyz;
        vNormal = mat3(transpose(inverse(uModel))) * aNormal;
        vTexCoord = aTexCoord;
        gl_Position = uProjection * uView * worldPos;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec3 vFragPos;
    in vec3 vNormal;
    in vec2 vTexCoord;
    out vec4 FragColor;
    
    uniform vec4 uColor;
    uniform vec3 uLightDir;
    uniform vec3 uLightColor;
    uniform vec3 uAmbient;
    uniform sampler2D uTexture;
    uniform bool uUseTexture;
    
    void main() {
        vec3 norm = normalize(vNormal);
        vec3 lightDir = normalize(uLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uLightColor;
        
        vec3 lighting = uAmbient + diffuse;
        vec4 baseColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);
        FragColor = vec4(lighting * baseColor.rgb * uColor.rgb, baseColor.a * uColor.a);
    }
)";

Shader* shader = new Shader();
shader->LoadFromSource(vertexSrc, fragmentSrc);
return shader;
```

}

Shader* ShaderPresets::CreatePBR() {
Logger::Warning(“PBR shader preset not yet implemented”);
return CreateLit();
}

Shader* ShaderPresets::CreateSkybox() {
const char* vertexSrc = R”(
#version 330 core
layout(location = 0) in vec3 aPos;

```
    uniform mat4 uView;
    uniform mat4 uProjection;
    
    out vec3 vTexCoord;
    
    void main() {
        vTexCoord = aPos;
        vec4 pos = uProjection * mat4(mat3(uView)) * vec4(aPos, 1.0);
        gl_Position = pos.xyww;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec3 vTexCoord;
    out vec4 FragColor;
    
    uniform samplerCube uSkybox;
    
    void main() {
        FragColor = texture(uSkybox, vTexCoord);
    }
)";

Shader* shader = new Shader();
shader->LoadFromSource(vertexSrc, fragmentSrc);
return shader;
```

}

Shader* ShaderPresets::CreateUI() {
const char* vertexSrc = R”(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in vec4 aColor;

```
    uniform mat4 uProjection;
    
    out vec2 vTexCoord;
    out vec4 vColor;
    
    void main() {
        gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
        vTexCoord = aTexCoord;
        vColor = aColor;
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    in vec4 vColor;
    out vec4 FragColor;
    
    uniform sampler2D uTexture;
    uniform bool uUseTexture;
    
    void main() {
        if (uUseTexture) {
            FragColor = texture(uTexture, vTexCoord) * vColor;
        } else {
            FragColor = vColor;
        }
    }
)";

Shader* shader = new Shader();
shader->LoadFromSource(vertexSrc, fragmentSrc);
return shader;
```

}

}
