/**
 * Titan Assets Header
 * 
 * Asset loading and management
 */

#ifndef TITAN_ASSETS_HPP
#define TITAN_ASSETS_HPP

#include "Titan_Graphics.hpp"

// Forward declaration for stb_truetype
struct stbtt_bakedchar;

namespace Titan::Assets {

// ============================================================================
// Asset Types
// ============================================================================

struct Font
{
    Graphics::TextureHandle atlas;
    uint32 width = 0;
    uint32 height = 0;
    float size = 16.0f;
    stbtt_bakedchar* glyphs = nullptr;
};

struct Texture
{
    Graphics::TextureHandle handle;
    uint32 width = 0;
    uint32 height = 0;
};

struct Material
{
    Graphics::ShaderHandle shader;
    Graphics::TextureHandle diffuse;
    
    void Bind();
    void Unbind();
};

// ============================================================================
// Asset Loader
// ============================================================================

struct Loader
{
    /**
     * Initialize the asset loading system
     */
    static void Init();
    
    /**
     * Shutdown and cleanup all loaded assets
     */
    static void Shutdown();
    
    /**
     * Load a texture from file
     * @param Path - Path to texture file
     * @param bPixelArt - If true, use nearest filtering
     */
    static Texture* LoadTexture(const char* Path, bool bPixelArt = false);
    
    /**
     * Load a font from file
     * @param Path - Path to TTF file
     * @param Size - Font size in pixels
     */
    static Font* LoadFont(const char* Path, float Size);
    
    /**
     * Create a sprite material
     * @param Tex - Texture to use
     */
    static Material* CreateSpriteMaterial(Texture* Tex);
    
    /**
     * Unload a texture
     */
    static void UnloadTexture(Texture* Tex);
    
    /**
     * Unload a font
     */
    static void UnloadFont(Font* Fnt);
};

} // namespace Titan::Assets

#endif // TITAN_ASSETS_HPP
