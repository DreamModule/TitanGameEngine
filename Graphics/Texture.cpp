#include “Texture.hpp”
#include “../Core/Logger.hpp”
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include “stb_image.h”

#ifdef _WIN32
#include <windows.h>
#include <gl/GL.h>
#ifndef APIENTRY
#define APIENTRY
#endif
#endif

#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif

#ifndef GL_RGB
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RED 0x1903
#define GL_RG 0x8227
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RGB8 0x8051
#define GL_RGBA8 0x8058
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_MIRRORED_REPEAT 0x8370
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE0 0x84C0
#endif

typedef void (APIENTRY* PFNGLGENTEXTURESPROC)(int32_t n, uint32_t* textures);
typedef void (APIENTRY* PFNGLBINDTEXTUREPROC)(uint32_t target, uint32_t texture);
typedef void (APIENTRY* PFNGLTEXIMAGE2DPROC)(uint32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void* pixels);
typedef void (APIENTRY* PFNGLTEXPARAMETERIPROC)(uint32_t target, uint32_t pname, int32_t param);
typedef void (APIENTRY* PFNGLGENERATEMIPMAPPROC)(uint32_t target);
typedef void (APIENTRY* PFNGLDELETETEXTURESPROC)(int32_t n, const uint32_t* textures);
typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(uint32_t texture);

namespace {
PFNGLGENTEXTURESPROC glGenTextures = nullptr;
PFNGLBINDTEXTUREPROC glBindTexture = nullptr;
PFNGLTEXIMAGE2DPROC glTexImage2D = nullptr;
PFNGLTEXPARAMETERIPROC glTexParameteri = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLDELETETEXTURESPROC glDeleteTextures = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;

```
bool g_glFunctionsLoaded = false;

void LoadGLFunctions() {
    if (g_glFunctionsLoaded) return;
```

#ifdef _WIN32
HMODULE opengl = LoadLibraryA(“opengl32.dll”);
if (opengl) {
glGenTextures = (PFNGLGENTEXTURESPROC)GetProcAddress(opengl, “glGenTextures”);
glBindTexture = (PFNGLBINDTEXTUREPROC)GetProcAddress(opengl, “glBindTexture”);
glTexImage2D = (PFNGLTEXIMAGE2DPROC)GetProcAddress(opengl, “glTexImage2D”);
glTexParameteri = (PFNGLTEXPARAMETERIPROC)GetProcAddress(opengl, “glTexParameteri”);
glDeleteTextures = (PFNGLDELETETEXTURESPROC)GetProcAddress(opengl, “glDeleteTextures”);

```
        glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    }
```

#endif

```
    g_glFunctionsLoaded = true;
}
```

}

namespace Titan {
namespace Graphics {

Texture::Texture() : textureId(0), width(0), height(0), format(TextureFormat::RGBA) {
LoadGLFunctions();
}

Texture::~Texture() {
if (textureId != 0 && glDeleteTextures) {
glDeleteTextures(1, &textureId);
}
}

bool Texture::LoadFromFile(const std::string& path, const TextureParams& params) {
if (params.flipVertically) {
stbi_set_flip_vertically_on_load(1);
} else {
stbi_set_flip_vertically_on_load(0);
}

```
int w, h, channels;
unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);

if (!data) {
    Logger::Error("Failed to load texture: " + path);
    return false;
}

TextureFormat fmt = TextureFormat::RGBA;
if (channels == 1) fmt = TextureFormat::R;
else if (channels == 2) fmt = TextureFormat::RG;
else if (channels == 3) fmt = TextureFormat::RGB;
else if (channels == 4) fmt = TextureFormat::RGBA;

bool success = LoadFromMemory(data, w, h, fmt, params);

stbi_image_free(data);

if (success) {
    Logger::Info("Loaded texture: " + path);
}

return success;
```

}

bool Texture::LoadFromMemory(const void* data, uint32_t w, uint32_t h,
TextureFormat fmt, const TextureParams& params) {
if (!glGenTextures || !glBindTexture || !glTexImage2D) {
Logger::Error(“OpenGL texture functions not loaded”);
return false;
}

```
if (textureId != 0) {
    glDeleteTextures(1, &textureId);
}

width = w;
height = h;
format = fmt;

glGenTextures(1, &textureId);
glBindTexture(GL_TEXTURE_2D, textureId);

uint32_t glFormat = GetGLFormat(format);
uint32_t glInternalFormat = GetGLInternalFormat(format);

glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, 
             glFormat, GL_UNSIGNED_BYTE, data);

ApplyParameters(params);

if (params.generateMipmaps && glGenerateMipmap) {
    glGenerateMipmap(GL_TEXTURE_2D);
}

glBindTexture(GL_TEXTURE_2D, 0);

return true;
```

}

void Texture::Bind(uint32_t slot) const {
if (textureId == 0) return;

```
if (glActiveTexture) {
    glActiveTexture(GL_TEXTURE0 + slot);
}

if (glBindTexture) {
    glBindTexture(GL_TEXTURE_2D, textureId);
}
```

}

void Texture::Unbind() const {
if (glBindTexture) {
glBindTexture(GL_TEXTURE_2D, 0);
}
}

void Texture::ApplyParameters(const TextureParams& params) {
if (!glTexParameteri) return;

```
auto GetGLFilter = [](TextureFilter filter) -> uint32_t {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }
};

auto GetGLWrap = [](TextureWrap wrap) -> uint32_t {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
};

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLFilter(params.minFilter));
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFilter(params.magFilter));
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrap(params.wrapS));
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrap(params.wrapT));
```

}

uint32_t Texture::GetGLFormat(TextureFormat fmt) {
switch (fmt) {
case TextureFormat::R: return GL_RED;
case TextureFormat::RG: return GL_RG;
case TextureFormat::RGB: return GL_RGB;
case TextureFormat::RGBA: return GL_RGBA;
case TextureFormat::Depth: return GL_DEPTH_COMPONENT;
default: return GL_RGBA;
}
}

uint32_t Texture::GetGLInternalFormat(TextureFormat fmt) {
switch (fmt) {
case TextureFormat::R: return GL_R8;
case TextureFormat::RG: return GL_RG8;
case TextureFormat::RGB: return GL_RGB8;
case TextureFormat::RGBA: return GL_RGBA8;
case TextureFormat::Depth: return GL_DEPTH_COMPONENT24;
default: return GL_RGBA8;
}
}

namespace TextureLibrary {
static std::unordered_map<std::string, Texture*> g_textures;

```
Texture* Load(const std::string& name, const std::string& path, const TextureParams& params) {
    auto it = g_textures.find(name);
    if (it != g_textures.end()) {
        return it->second;
    }
    
    Texture* texture = new Texture();
    if (texture->LoadFromFile(path, params)) {
        g_textures[name] = texture;
        return texture;
    }
    
    delete texture;
    return nullptr;
}

Texture* Get(const std::string& name) {
    auto it = g_textures.find(name);
    if (it != g_textures.end()) {
        return it->second;
    }
    return nullptr;
}

void Clear() {
    for (auto& pair : g_textures) {
        delete pair.second;
    }
    g_textures.clear();
}
```

}

}
}
