#ifndef TITAN_ASSETS_HPP
#define TITAN_ASSETS_HPP

#include "Titan_Graphics.hpp"

struct stbtt_bakedchar;

namespace Titan::Assets {

struct Font {
    Graphics::TextureHandle atlas;
    u32 width, height;
    f32 size;
    stbtt_bakedchar* glyphs;
};

struct Texture {
    Graphics::TextureHandle handle;
    u32 width, height;
};

struct Material {
    Graphics::ShaderHandle shader;
    Graphics::TextureHandle diffuse;
    void Bind();
};

struct Loader {
    static void Init();
    static Texture* LoadTexture(const char* p, bool pixel);
    static Font* LoadFont(const char* p, f32 size);
    static Material* CreateSpriteMaterial(Texture* t);
};
}
#endif
