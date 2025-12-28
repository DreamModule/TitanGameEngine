#pragma once

#include <string>
#include <cstdint>

namespace Titan {
namespace Graphics {

enum class TextureFormat {
RGB,
RGBA,
R,
RG,
Depth
};

enum class TextureFilter {
Nearest,
Linear,
NearestMipmapNearest,
LinearMipmapNearest,
NearestMipmapLinear,
LinearMipmapLinear
};

enum class TextureWrap {
Repeat,
MirroredRepeat,
ClampToEdge,
ClampToBorder
};

struct TextureParams {
TextureFilter minFilter = TextureFilter::Linear;
TextureFilter magFilter = TextureFilter::Linear;
TextureWrap wrapS = TextureWrap::Repeat;
TextureWrap wrapT = TextureWrap::Repeat;
bool generateMipmaps = true;
bool flipVertically = true;
};

class Texture {
public:
Texture();
~Texture();

```
bool LoadFromFile(const std::string& path, const TextureParams& params = TextureParams());
bool LoadFromMemory(const void* data, uint32_t width, uint32_t height, 
                   TextureFormat format, const TextureParams& params = TextureParams());

void Bind(uint32_t slot = 0) const;
void Unbind() const;

uint32_t GetTextureId() const { return textureId; }
uint32_t GetWidth() const { return width; }
uint32_t GetHeight() const { return height; }
TextureFormat GetFormat() const { return format; }

bool IsValid() const { return textureId != 0; }
```

private:
uint32_t textureId;
uint32_t width;
uint32_t height;
TextureFormat format;

```
void ApplyParameters(const TextureParams& params);
uint32_t GetGLFormat(TextureFormat fmt);
uint32_t GetGLInternalFormat(TextureFormat fmt);
```

};

namespace TextureLibrary {
Texture* Load(const std::string& name, const std::string& path, const TextureParams& params = TextureParams());
Texture* Get(const std::string& name);
void Clear();
}

}
}
