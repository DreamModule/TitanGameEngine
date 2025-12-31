#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Loader.hpp"
#include <gl/GL.h>
#include <string>
#include <unordered_map>

namespace {
    std::unordered_map<std::string, Titan::Assets::Texture> g_textureCache;
}

namespace Titan::Assets {

Texture Loader::LoadTexture(const char* path) {
    std::string key(path);
    auto it = g_textureCache.find(key);
    if (it != g_textureCache.end()) return it->second;
    int w,h,n;
    unsigned char* data = stbi_load(path, &w, &h, &n, 4);
    Texture t{};
    if (data) {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        t.id = tex;
        t.width = w;
        t.height = h;
        g_textureCache.emplace(key, t);
    }
    return t;
}

}
