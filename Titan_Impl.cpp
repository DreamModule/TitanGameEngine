// !!! ВАЖНО: Требуется файл miniaudio.h в папке проекта !!!
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h" 

#include "Titan_Engine.hpp"
#include "Titan_Platform_Win32.cpp" 
#include "Titan_Audio.cpp"         
#include <gl/GL.h>
#include <unordered_map>

namespace Titan::Graphics {
    struct GLBuffer { u32 id; std::vector<VertexAttribute> layout; u32 stride; };
    std::unordered_map<u32, GLBuffer> g_BufferMap;
    class GLRD : public IRenderDevice {
        void Init() override { glEnable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); }
        void Shutdown() override {}
        BufferHandle CreateBuffer(const BufferDesc& d) override { 
            u32 i; glGenBuffers(1,&i); glBindBuffer(d.type==BufferType::Vertex?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER,i); 
            glBufferData(d.type==BufferType::Vertex?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER,d.size,d.data,d.usage==BufferUsage::Static?GL_STATIC_DRAW:GL_DYNAMIC_DRAW); 
            u32 s=0; for(auto& a:d.layout)s+=a.count*4; g_BufferMap[i]={i,d.layout,s}; return {i}; 
        }
        void UpdateBuffer(BufferHandle h, usize o, usize s, const void* d) override { glBindBuffer(GL_ARRAY_BUFFER,h.id); glBufferSubData(GL_ARRAY_BUFFER,o,s,d); }
        void DestroyBuffer(BufferHandle h) override { glDeleteBuffers(1,&h.id); g_BufferMap.erase(h.id); }
        void DestroyTexture(TextureHandle h) override { glDeleteTextures(1,&h.id); }
        ShaderHandle CreateShader(const char* v, const char* f) override {
            auto c=[](u32 t,const char* s){u32 i=glCreateShader(t);glShaderSource(i,1,&s,0);glCompileShader(i);return i;};
            u32 vs=c(GL_VERTEX_SHADER,v), fs=c(GL_FRAGMENT_SHADER,f), p=glCreateProgram();
            glAttachShader(p,vs); glAttachShader(p,fs); glLinkProgram(p); glDeleteShader(vs); glDeleteShader(fs); return {p};
        }
        void DestroyShader(ShaderHandle s) override { glDeleteShader(s.id); }
        TextureHandle CreateTexture(const TextureDesc& d) override {
            u32 i; glGenTextures(1,&i); glBindTexture(GL_TEXTURE_2D,i);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,d.filter==TextureFilter::Nearest?GL_NEAREST:GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,d.filter==TextureFilter::Nearest?GL_NEAREST:GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D,0,d.format==TextureFormat::R8?GL_RED:GL_RGBA,d.width,d.height,0,d.format==TextureFormat::R8?GL_RED:GL_RGBA,GL_UNSIGNED_BYTE,d.data);
            return {i};
        }
        void BindPipeline(ShaderHandle s) override { glUseProgram(s.id); }
        void BindVertexBuffer(BufferHandle h) override { 
            auto it=g_BufferMap.find(h.id); if(it!=g_BufferMap.end()){
                const GLBuffer& b=it->second; glBindBuffer(GL_ARRAY_BUFFER,h.id); u32 off=0;
                for(u32 i=0;i<b.layout.size();++i){glVertexAttribPointer(i,b.layout[i].count,GL_FLOAT,0,b.stride,(void*)(u64)off); glEnableVertexAttribArray(i); off+=b.layout[i].count*4;}
            }
        }
        void BindIndexBuffer(BufferHandle h) override { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,h.id); }
        void BindTexture(TextureHandle h, u32 s) override { glActiveTexture(GL_TEXTURE0+s); glBindTexture(GL_TEXTURE_2D,h.id); }
        void SetUniformMat4(ShaderHandle s, const char* n, const Math::Mat4& m) override { glUniformMatrix4fv(glGetUniformLocation(s.id,n),1,GL_TRUE,&m.m[0][0]); }
        void SetUniformVec4(ShaderHandle s, const char* n, const Math::Vec4& v) override { glUniform4f(glGetUniformLocation(s.id,n),v.x,v.y,v.z,v.w); }
        void SetUniformInt(ShaderHandle s, const char* n, i32 v) override { glUniform1i(glGetUniformLocation(s.id,n),v); }
        void Clear(f32 r, f32 g, f32 b) override { glClearColor(r,g,b,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }
        void DrawIndexed(u32 c, u32 o) override { glDrawElements(GL_TRIANGLES,c,GL_UNSIGNED_INT,(void*)(u64)(o*4)); }
        void DrawArrays(u32 c, u32 f) override { glDrawArrays(GL_TRIANGLES, f, c); }
    };
    IRenderDevice* g_Device = nullptr;
    void Backend::Init() { g_Device=new GLRD(); g_Device->Init(); }
    void Backend::Shutdown() { if(g_Device){g_Device->Shutdown(); delete g_Device;} }
    void Backend::Clear(f32 r, f32 g, f32 b) { g_Device->Clear(r,g,b); }
}

// ENGINE IMPL
namespace Titan {
    static Engine::Context gCtx; Engine::Context* Engine::Get() { return &gCtx; }
    void Engine::Init(const char* t, u32 w, u32 h) {
        gCtx.isRunning = true;
        Platform::Init(w, h, t); Graphics::Backend::Init(); Assets::Loader::Init();
        gCtx.events.Init(); gCtx.input.Init(); gCtx.audio.Init(); gCtx.stateMgr.Init(&gCtx); Debug::Init();
        gCtx.world.Clear(); gCtx.scheduler.Init(gCtx.world);
    }
    void Engine::Shutdown() {
        gCtx.scheduler.Shutdown(gCtx.world); gCtx.audio.Shutdown(); gCtx.events.Shutdown();
        Graphics::Backend::Shutdown(); Platform::Shutdown();
    }
}
