#ifndef TITAN_GRAPHICS_HPP
#define TITAN_GRAPHICS_HPP

#include "Titan_Core.hpp"

namespace Titan::Graphics {

struct Handle { u32 id; bool IsValid() const { return id!=0; } };
struct BufferHandle : Handle {};
struct ShaderHandle : Handle {};
struct TextureHandle : Handle {};

enum class BufferType { Vertex, Index };
enum class BufferUsage { Static, Dynamic };
enum class TextureFormat { R8, RGBA8 };
enum class TextureFilter { Nearest, Linear };

struct BufferDesc { BufferType type; BufferUsage usage; usize size; const void* data; };
struct TextureDesc { u32 width; u32 height; TextureFormat format; TextureFilter filter; const void* data; };

class IRenderDevice {
public:
    virtual ~IRenderDevice() {}
    virtual void Init()=0;
    virtual void Shutdown()=0;
    virtual BufferHandle CreateBuffer(const BufferDesc& d)=0;
    virtual void UpdateBuffer(BufferHandle h, usize o, usize s, const void* d)=0;
    virtual void DestroyBuffer(BufferHandle h)=0;
    virtual ShaderHandle CreateShader(const char* v, const char* f)=0;
    virtual TextureHandle CreateTexture(const TextureDesc& d)=0;
    virtual void BindPipeline(ShaderHandle s)=0;
    virtual void BindVertexBuffer(BufferHandle h)=0;
    virtual void BindIndexBuffer(BufferHandle h)=0;
    virtual void BindTexture(TextureHandle h, u32 s)=0;
    virtual void SetUniformMat4(ShaderHandle s, const char* n, const Math::Mat4& m)=0;
    virtual void SetUniformVec4(ShaderHandle s, const char* n, const Math::Vec4& v)=0;
    virtual void SetUniformInt(ShaderHandle s, const char* n, i32 v)=0;
    virtual void Clear(f32 r, f32 g, f32 b)=0;
    virtual void DrawIndexed(u32 c, u32 o)=0;
};

extern IRenderDevice* g_Device;

struct Backend {
    static void Init();
    static void Clear(f32 r, f32 g, f32 b);
};

struct Camera {
    static Math::Mat4 GetViewProjection();
    static void SetPerspective(f32 fov, f32 r);
    static void LookAt(const Math::Vec3& e, const Math::Vec3& t);
};

namespace UI {
    struct Context { ShaderHandle s; BufferHandle v; BufferHandle i; Math::Mat4 p; };
    extern Context g_UI;
    void Init(u32 w, u32 h);
}
}
#endif
