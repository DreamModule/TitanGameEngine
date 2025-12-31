/**
 * Titan Engine Implementation
 * 
 * OpenGL Render Device and Engine Context
 */

// NOTE: Requires miniaudio.h in project folder
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h" 

#include "Titan_Engine.hpp"
#include "Titan_Platform_Win32.cpp" 
#include "Titan_Audio.cpp"         

#ifdef _WIN32
    #include <windows.h>
#include <gl/GL.h>
    
    // OpenGL function declarations
    typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei, GLuint*);
    typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint);
    typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint*);
    typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
    typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
    typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum, GLuint);
    typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void*, GLenum);
    typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum, GLintptr, GLsizeiptr, const void*);
    typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
    typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
    typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC)(GLenum);
    typedef void (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar**, const GLint*);
    typedef void (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint);
    typedef void (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
    typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
    typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC)();
    typedef void (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint, GLuint);
    typedef void (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint);
    typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
    typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
    typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint);
    typedef void (APIENTRY* PFNGLDELETESHADERPROC)(GLuint);
    typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint);
    typedef GLint (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
    typedef void (APIENTRY* PFNGLUNIFORM1IPROC)(GLint, GLint);
    typedef void (APIENTRY* PFNGLUNIFORM4FPROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat*);
    typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum);

    // OpenGL constants
    #ifndef GL_ARRAY_BUFFER
        #define GL_ARRAY_BUFFER 0x8892
        #define GL_ELEMENT_ARRAY_BUFFER 0x8893
        #define GL_STATIC_DRAW 0x88E4
        #define GL_DYNAMIC_DRAW 0x88E8
        #define GL_FRAGMENT_SHADER 0x8B30
        #define GL_VERTEX_SHADER 0x8B31
        #define GL_COMPILE_STATUS 0x8B81
        #define GL_LINK_STATUS 0x8B82
        #define GL_TEXTURE0 0x84C0
    #endif
#endif

#include <unordered_map>
#include <cstdio>

namespace Titan::Graphics {

// ============================================================================
// OpenGL Function Pointers
// ============================================================================

static PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
static PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
static PFNGLBUFFERDATAPROC glBufferData = nullptr;
static PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
static PFNGLCREATESHADERPROC glCreateShader = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader = nullptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
static PFNGLUNIFORM1IPROC glUniform1i = nullptr;
static PFNGLUNIFORM4FPROC glUniform4f = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
static PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;

static bool GOpenGLLoaded = false;

static bool LoadOpenGLFunctions()
{
    if (GOpenGLLoaded) return true;

#ifdef _WIN32
    #define LOAD_GL_FUNC(Name) Name = (decltype(Name))wglGetProcAddress(#Name)
    
    LOAD_GL_FUNC(glGenBuffers);
    LOAD_GL_FUNC(glDeleteBuffers);
    LOAD_GL_FUNC(glBindBuffer);
    LOAD_GL_FUNC(glBufferData);
    LOAD_GL_FUNC(glBufferSubData);
    LOAD_GL_FUNC(glVertexAttribPointer);
    LOAD_GL_FUNC(glEnableVertexAttribArray);
    LOAD_GL_FUNC(glCreateShader);
    LOAD_GL_FUNC(glShaderSource);
    LOAD_GL_FUNC(glCompileShader);
    LOAD_GL_FUNC(glGetShaderiv);
    LOAD_GL_FUNC(glGetShaderInfoLog);
    LOAD_GL_FUNC(glCreateProgram);
    LOAD_GL_FUNC(glAttachShader);
    LOAD_GL_FUNC(glLinkProgram);
    LOAD_GL_FUNC(glGetProgramiv);
    LOAD_GL_FUNC(glGetProgramInfoLog);
    LOAD_GL_FUNC(glUseProgram);
    LOAD_GL_FUNC(glDeleteShader);
    LOAD_GL_FUNC(glDeleteProgram);
    LOAD_GL_FUNC(glGetUniformLocation);
    LOAD_GL_FUNC(glUniform1i);
    LOAD_GL_FUNC(glUniform4f);
    LOAD_GL_FUNC(glUniformMatrix4fv);
    LOAD_GL_FUNC(glActiveTexture);
    
    #undef LOAD_GL_FUNC
#endif

    GOpenGLLoaded = true;
    return true;
}

// ============================================================================
// Buffer Storage
// ============================================================================

struct FGLBuffer
{
    uint32 ID;
    std::vector<VertexAttribute> Layout;
    uint32 Stride;
};

static std::unordered_map<uint32, FGLBuffer> GBufferMap;

// ============================================================================
// Error Checking
// ============================================================================

static bool CheckShaderCompileError(uint32 ShaderID, const char* Type)
{
    GLint Success;
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Success);
    
    if (!Success)
    {
        char InfoLog[512];
        glGetShaderInfoLog(ShaderID, 512, nullptr, InfoLog);
        fprintf(stderr, "[TitanEngine] %s Shader Compile Error: %s\n", Type, InfoLog);
        return false;
    }
    return true;
}

static bool CheckProgramLinkError(uint32 ProgramID)
{
    GLint Success;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Success);
    
    if (!Success)
    {
        char InfoLog[512];
        glGetProgramInfoLog(ProgramID, 512, nullptr, InfoLog);
        fprintf(stderr, "[TitanEngine] Shader Program Link Error: %s\n", InfoLog);
        return false;
    }
    return true;
}

static void CheckGLError(const char* Operation)
{
    GLenum Error = glGetError();
    if (Error != GL_NO_ERROR)
    {
        fprintf(stderr, "[TitanEngine] OpenGL Error during %s: 0x%X\n", Operation, Error);
    }
}

// ============================================================================
// OpenGL Render Device
// ============================================================================

class FOpenGLRenderDevice : public IRenderDevice
{
public:
    void Init() override
    {
        LoadOpenGLFunctions();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        CheckGLError("Init");
    }

    void Shutdown() override
    {
        GBufferMap.clear();
    }

    BufferHandle CreateBuffer(const BufferDesc& Desc) override
    {
        uint32 BufferID;
        glGenBuffers(1, &BufferID);
        
        GLenum Target = (Desc.type == BufferType::Vertex) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
        GLenum Usage = (Desc.usage == BufferUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
        
        glBindBuffer(Target, BufferID);
        glBufferData(Target, static_cast<GLsizeiptr>(Desc.size), Desc.data, Usage);
        
        // Calculate stride
        uint32 Stride = 0;
        for (const auto& Attr : Desc.layout)
        {
            Stride += Attr.count * 4; // sizeof(float)
        }
        
        GBufferMap[BufferID] = {BufferID, Desc.layout, Stride};
        
        CheckGLError("CreateBuffer");
        return {BufferID};
    }

    void UpdateBuffer(BufferHandle Handle, usize Offset, usize Size, const void* Data) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, Handle.id);
        glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(Offset), static_cast<GLsizeiptr>(Size), Data);
        CheckGLError("UpdateBuffer");
    }

    void DestroyBuffer(BufferHandle Handle) override
    {
        glDeleteBuffers(1, &Handle.id);
        GBufferMap.erase(Handle.id);
    }

    ShaderHandle CreateShader(const char* VertexSource, const char* FragmentSource) override
    {
        // Compile vertex shader
        uint32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShader, 1, &VertexSource, nullptr);
        glCompileShader(VertexShader);
        
        if (!CheckShaderCompileError(VertexShader, "Vertex"))
        {
            glDeleteShader(VertexShader);
            return {0};
        }

        // Compile fragment shader
        uint32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShader, 1, &FragmentSource, nullptr);
        glCompileShader(FragmentShader);
        
        if (!CheckShaderCompileError(FragmentShader, "Fragment"))
        {
            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
            return {0};
        }

        // Link program
        uint32 Program = glCreateProgram();
        glAttachShader(Program, VertexShader);
        glAttachShader(Program, FragmentShader);
        glLinkProgram(Program);

        if (!CheckProgramLinkError(Program))
        {
            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
            glDeleteProgram(Program);
            return {0};
        }

        // Cleanup shaders (they're linked to program now)
        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        return {Program};
    }

    void DestroyShader(ShaderHandle Handle) override
    {
        glDeleteProgram(Handle.id);
    }

    TextureHandle CreateTexture(const TextureDesc& Desc) override
    {
        uint32 TextureID;
        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        GLint MinFilter = (Desc.filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
        GLint MagFilter = (Desc.filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        GLint InternalFormat = (Desc.format == TextureFormat::R8) ? GL_RED : GL_RGBA;
        GLenum Format = (Desc.format == TextureFormat::R8) ? GL_RED : GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, 
                     static_cast<GLsizei>(Desc.width), 
                     static_cast<GLsizei>(Desc.height), 
                     0, Format, GL_UNSIGNED_BYTE, Desc.data);

        CheckGLError("CreateTexture");
        return {TextureID};
    }

    void DestroyTexture(TextureHandle Handle) override
    {
        glDeleteTextures(1, &Handle.id);
    }

    void BindPipeline(ShaderHandle Handle) override
    {
        glUseProgram(Handle.id);
    }

    void BindVertexBuffer(BufferHandle Handle) override
    {
        auto It = GBufferMap.find(Handle.id);
        if (It == GBufferMap.end())
        {
            return;
        }

        const FGLBuffer& Buffer = It->second;
        glBindBuffer(GL_ARRAY_BUFFER, Handle.id);

        uint32 Offset = 0;
        for (uint32 i = 0; i < Buffer.Layout.size(); ++i)
        {
            glVertexAttribPointer(
                i,
                static_cast<GLint>(Buffer.Layout[i].count),
                GL_FLOAT,
                Buffer.Layout[i].normalized ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(Buffer.Stride),
                reinterpret_cast<void*>(static_cast<uintptr_t>(Offset))
            );
            glEnableVertexAttribArray(i);
            Offset += Buffer.Layout[i].count * 4;
        }
    }

    void BindIndexBuffer(BufferHandle Handle) override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Handle.id);
    }

    void BindTexture(TextureHandle Handle, uint32 Slot) override
    {
        glActiveTexture(GL_TEXTURE0 + Slot);
        glBindTexture(GL_TEXTURE_2D, Handle.id);
    }

    void SetUniformMat4(ShaderHandle Handle, const char* Name, const Math::Mat4& Matrix) override
    {
        GLint Location = glGetUniformLocation(Handle.id, Name);
        if (Location != -1)
        {
            glUniformMatrix4fv(Location, 1, GL_TRUE, &Matrix.m[0][0]);
        }
    }

    void SetUniformVec4(ShaderHandle Handle, const char* Name, const Math::Vec4& Vector) override
    {
        GLint Location = glGetUniformLocation(Handle.id, Name);
        if (Location != -1)
        {
            glUniform4f(Location, Vector.x, Vector.y, Vector.z, Vector.w);
        }
    }

    void SetUniformInt(ShaderHandle Handle, const char* Name, int32 Value) override
    {
        GLint Location = glGetUniformLocation(Handle.id, Name);
        if (Location != -1)
        {
            glUniform1i(Location, Value);
        }
    }

    void Clear(float R, float G, float B) override
    {
        glClearColor(R, G, B, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void DrawIndexed(uint32 Count, uint32 Offset) override
    {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Count), GL_UNSIGNED_INT, 
                       reinterpret_cast<void*>(static_cast<uintptr_t>(Offset * 4)));
    }

    void DrawArrays(uint32 Count, uint32 First) override
    {
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(First), static_cast<GLsizei>(Count));
    }
};

// ============================================================================
// Global Device
// ============================================================================

    IRenderDevice* g_Device = nullptr;

void Backend::Init()
{
    g_Device = new FOpenGLRenderDevice();
    g_Device->Init();
}

void Backend::Shutdown()
{
    if (g_Device)
    {
        g_Device->Shutdown();
        delete g_Device;
        g_Device = nullptr;
    }
}

void Backend::Clear(float R, float G, float B)
{
    if (g_Device)
    {
        g_Device->Clear(R, G, B);
    }
}

} // namespace Titan::Graphics

// ============================================================================
// Engine Implementation
// ============================================================================

namespace Titan {

static Engine::Context GEngineContext;

Engine::Context* Engine::Get()
{
    return &GEngineContext;
}

void Engine::Init(const char* Title, uint32 Width, uint32 Height)
{
    GEngineContext.isRunning = true;
    
    Platform::Init(Width, Height, Title);
    Graphics::Backend::Init();
    Assets::Loader::Init();
    
    GEngineContext.events.Init();
    GEngineContext.input.Init();
    GEngineContext.audio.Init();
    GEngineContext.stateMgr.Init(&GEngineContext);
    
    Debug::Initialize();
    
    GEngineContext.world.Clear();
    GEngineContext.scheduler.Init(GEngineContext.world);
}

void Engine::Shutdown()
{
    GEngineContext.scheduler.Shutdown(GEngineContext.world);
    
    Debug::Shutdown();
    
    GEngineContext.audio.Shutdown();
    GEngineContext.events.Shutdown();
    
    Graphics::Backend::Shutdown();
    Platform::Shutdown();
}

} // namespace Titan
