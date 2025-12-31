/**
 * Titan Graphics Header
 * 
 * Graphics device abstraction and rendering
 */

#ifndef TITAN_GRAPHICS_HPP
#define TITAN_GRAPHICS_HPP

#include "Titan_Core.hpp"
#include "Titan_Math.hpp"
#include <vector>

namespace Titan::Graphics {

// ============================================================================
// Handle Types
// ============================================================================

struct Handle
{
    uint32 id = 0;
    bool IsValid() const { return id != 0; }
};

struct BufferHandle : Handle {};
struct ShaderHandle : Handle {};
struct TextureHandle : Handle {};

// ============================================================================
// Enums
// ============================================================================

enum class BufferType
{
    Vertex,
    Index
};

enum class BufferUsage
{
    Static,
    Dynamic
};

enum class AttributeType
{
    Float,
    Byte,
    UByte
};

enum class TextureFormat
{
    R8,
    RGBA8,
    RGB8,
    Depth24
};

enum class TextureFilter
{
    Nearest,
    Linear
};

// ============================================================================
// Descriptors
// ============================================================================

struct VertexAttribute
{
    uint32 count;
    AttributeType type;
    bool normalized;
};

struct BufferDesc
{
    BufferType type;
    BufferUsage usage;
    usize size;
    const void* data;
    std::vector<VertexAttribute> layout;
};

struct TextureDesc
{
    uint32 width;
    uint32 height;
    TextureFormat format;
    TextureFilter filter;
    const void* data;
};

// ============================================================================
// Render Device Interface
// ============================================================================

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;
    
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    
    // Buffer operations
    virtual BufferHandle CreateBuffer(const BufferDesc& Desc) = 0;
    virtual void UpdateBuffer(BufferHandle Handle, usize Offset, usize Size, const void* Data) = 0;
    virtual void DestroyBuffer(BufferHandle Handle) = 0;
    
    // Shader operations
    virtual ShaderHandle CreateShader(const char* VertexSource, const char* FragmentSource) = 0;
    virtual void DestroyShader(ShaderHandle Handle) = 0;
    
    // Texture operations
    virtual TextureHandle CreateTexture(const TextureDesc& Desc) = 0;
    virtual void DestroyTexture(TextureHandle Handle) = 0;
    
    // Binding
    virtual void BindPipeline(ShaderHandle Handle) = 0;
    virtual void BindVertexBuffer(BufferHandle Handle) = 0;
    virtual void BindIndexBuffer(BufferHandle Handle) = 0;
    virtual void BindTexture(TextureHandle Handle, uint32 Slot) = 0;
    
    // Uniforms
    virtual void SetUniformMat4(ShaderHandle Handle, const char* Name, const Math::Mat4& Matrix) = 0;
    virtual void SetUniformVec4(ShaderHandle Handle, const char* Name, const Math::Vec4& Vector) = 0;
    virtual void SetUniformInt(ShaderHandle Handle, const char* Name, int32 Value) = 0;
    
    // Drawing
    virtual void Clear(float R, float G, float B) = 0;
    virtual void DrawIndexed(uint32 Count, uint32 Offset) = 0;
    virtual void DrawArrays(uint32 Count, uint32 First) = 0;
};

// ============================================================================
// Global Device
// ============================================================================

extern IRenderDevice* g_Device;

// ============================================================================
// Backend Utilities
// ============================================================================

struct Backend
{
    static void Init();
    static void Shutdown();
    static void Clear(float R, float G, float B);
};

} // namespace Titan::Graphics

#endif // TITAN_GRAPHICS_HPP
