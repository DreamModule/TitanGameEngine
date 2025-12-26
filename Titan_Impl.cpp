#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "Titan_Core.hpp"
#include "Titan_Events.hpp"
#include "Titan_Input.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_Assets.hpp"
#include "Titan_Window.hpp"
#include "Titan_ECS.hpp"
#include "Titan_State.hpp"
#include "Titan_Engine.hpp"

namespace Titan {

// Events
std::unordered_map<EventID, std::vector<EventHandler>> EventBus::globalListeners;
std::unordered_map<EventID, std::vector<EventHandler>> EventBus::stateListeners;
void EventBus::Init() { globalListeners.clear(); stateListeners.clear(); }
void EventBus::Subscribe(EventID id, EventHandler h, EventScope s) {
    if(s==EventScope::Global) globalListeners[id].push_back(h);
    else stateListeners[id].push_back(h);
}
void EventBus::Emit(EventID id, const EventContext& ctx) {
    if(globalListeners.count(id)) for(auto& h:globalListeners[id]) h(ctx);
    if(stateListeners.count(id)) for(auto& h:stateListeners[id]) h(ctx);
}
void EventBus::ResetState() { stateListeners.clear(); }
void EventBus::Shutdown() { globalListeners.clear(); stateListeners.clear(); }

// Window (Win32)
#ifdef _WIN32
static HWND g_Hwnd; static HDC g_HDC; static HGLRC g_HGLRC; static bool g_Run;
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) { if(m==WM_CLOSE) g_Run=false; return DefWindowProc(h,m,w,l); }
void Window::System::Init(u32 w, u32 h, const char* t) {
    WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=GetModuleHandle(0); wc.lpszClassName="T"; RegisterClass(&wc);
    g_Hwnd=CreateWindow("T",t,WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,w,h,0,0,wc.hInstance,0);
    g_HDC=GetDC(g_Hwnd);
    PIXELFORMATDESCRIPTOR p={sizeof(p),1,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,32,0,0,0,0,0,0,0,0,0,0,0,0,0,24,8,0,0,0,0,0,0};
    SetPixelFormat(g_HDC,ChoosePixelFormat(g_HDC,&p),&p);
    g_HGLRC=wglCreateContext(g_HDC); wglMakeCurrent(g_HDC,g_HGLRC); g_Run=true;
}
bool Window::System::Update() { MSG m; while(PeekMessage(&m,0,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessage(&m);} return g_Run; }
void Window::System::SwapBuffers() { ::SwapBuffers(g_HDC); }
void Window::System::Shutdown() { wglDeleteContext(g_HGLRC); DestroyWindow(g_Hwnd); }
#endif

// Input
struct Bind { u32 h; int t; KeyCode s, n; f32 sc; };
struct Ax { f32 v, inj; }; struct Ac { bool p, jp; };
std::vector<Bind> g_Binds; std::unordered_map<u32,Ax> g_Ax; std::unordered_map<u32,Ac> g_Ac;
Math::Vec2 g_M;
void Input::Manager::Init() {}
void Input::Manager::MapAction(const char* n, KeyCode k) { g_Binds.push_back({Data::Hash::FNV1a_32(n),0,k,KeyCode::None,1}); }
void Input::Manager::MapAxisKeys(const char* n, KeyCode p, KeyCode ng) { g_Binds.push_back({Data::Hash::FNV1a_32(n),1,p,ng,1}); }
void Input::Manager::MapAxisDirect(const char* n, KeyCode a, f32 s) { g_Binds.push_back({Data::Hash::FNV1a_32(n),2,a,KeyCode::None,s}); }
void Input::Manager::InjectAxis(const char* n, f32 v) { g_Ax[Data::Hash::FNV1a_32(n)].inj = v; }
Math::Vec2 Input::Manager::GetMousePosition() { return g_M; }
bool Input::Manager::GetActionDown(const char* n) { return g_Ac[Data::Hash::FNV1a_32(n)].jp; }
f32 Input::Manager::GetAxis(const char* n) { return g_Ax[Data::Hash::FNV1a_32(n)].v; }
void Input::Manager::Update() {
    POINT p; GetCursorPos(&p); ScreenToClient(g_Hwnd, &p); g_M={(f32)p.x,(f32)p.y};
    for(auto& kv:g_Ac) kv.second.jp=false;
    for(auto& kv:g_Ax) { kv.second.v=kv.second.inj; kv.second.inj=0; }
    auto K = [](KeyCode k) {
        int v=0;
        if(k==KeyCode::W)v=0x57; else if(k==KeyCode::A)v=0x41; else if(k==KeyCode::S)v=0x53; else if(k==KeyCode::D)v=0x44;
        else if(k==KeyCode::Space)v=VK_SPACE; else if(k==KeyCode::MouseLeft)v=VK_LBUTTON;
        return (GetAsyncKeyState(v)&0x8000)!=0;
    };
    for(auto& b:g_Binds) {
        if(b.t==0) { bool d=K(b.s); if(d && !g_Ac[b.h].p) g_Ac[b.h].jp=true; g_Ac[b.h].p=d; }
        else if(b.t==1) { if(K(b.s)) g_Ax[b.h].v+=1; if(K(b.n)) g_Ax[b.h].v-=1; }
    }
}

// Graphics
IRenderDevice* Graphics::g_Device = nullptr;
namespace Graphics {
    class GLRD : public IRenderDevice {
        void Init() override { glEnable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); }
        void Shutdown() override {}
        BufferHandle CreateBuffer(const BufferDesc& d) override { u32 i; glGenBuffers(1,&i); glBindBuffer(d.type==BufferType::Vertex?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER,i); glBufferData(d.type==BufferType::Vertex?GL_ARRAY_BUFFER:GL_ELEMENT_ARRAY_BUFFER,d.size,d.data,GL_STATIC_DRAW); return {i}; }
        void UpdateBuffer(BufferHandle h, usize o, usize s, const void* d) override { glBindBuffer(GL_ARRAY_BUFFER,h.id); glBufferSubData(GL_ARRAY_BUFFER,o,s,d); }
        void DestroyBuffer(BufferHandle h) override { glDeleteBuffers(1,&h.id); }
        ShaderHandle CreateShader(const char* v, const char* f) override {
            auto c=[](u32 t,const char* s){u32 i=glCreateShader(t);glShaderSource(i,1,&s,0);glCompileShader(i);return i;};
            u32 vs=c(GL_VERTEX_SHADER,v), fs=c(GL_FRAGMENT_SHADER,f), p=glCreateProgram();
            glAttachShader(p,vs); glAttachShader(p,fs); glLinkProgram(p); glDeleteShader(vs); glDeleteShader(fs); return {p};
        }
        TextureHandle CreateTexture(const TextureDesc& d) override {
            u32 i; glGenTextures(1,&i); glBindTexture(GL_TEXTURE_2D,i);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,d.filter==TextureFilter::Nearest?GL_NEAREST:GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,d.filter==TextureFilter::Nearest?GL_NEAREST:GL_LINEAR);
            u32 fmt = d.format==TextureFormat::R8?GL_RED:GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D,0,fmt,d.width,d.height,0,fmt,GL_UNSIGNED_BYTE,d.data);
            if(d.format==TextureFormat::R8) { GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED}; glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask); }
            return {i};
        }
        void BindPipeline(ShaderHandle s) override { glUseProgram(s.id); }
        void BindVertexBuffer(BufferHandle h) override { glBindBuffer(GL_ARRAY_BUFFER,h.id); glVertexAttribPointer(0,3,GL_FLOAT,0,5*4,0); glEnableVertexAttribArray(0); glVertexAttribPointer(1,2,GL_FLOAT,0,5*4,(void*)(3*4)); glEnableVertexAttribArray(1); }
        void BindIndexBuffer(BufferHandle h) override { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,h.id); }
        void BindTexture(TextureHandle h, u32 s) override { glActiveTexture(GL_TEXTURE0+s); glBindTexture(GL_TEXTURE_2D,h.id); }
        void SetUniformMat4(ShaderHandle s, const char* n, const Math::Mat4& m) override { glUniformMatrix4fv(glGetUniformLocation(s.id,n),1,GL_TRUE,&m.m[0][0]); }
        void SetUniformVec4(ShaderHandle s, const char* n, const Math::Vec4& v) override { glUniform4f(glGetUniformLocation(s.id,n),v.x,v.y,v.z,v.w); }
        void SetUniformInt(ShaderHandle s, const char* n, i32 v) override { glUniform1i(glGetUniformLocation(s.id,n),v); }
        void Clear(f32 r, f32 g, f32 b) override { glClearColor(r,g,b,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }
        void DrawIndexed(u32 c, u32 o) override { glDrawElements(GL_TRIANGLES,c,GL_UNSIGNED_INT,(void*)(u64)(o*4)); }
    };
    namespace UI { Context g_UI; void Init(u32 w, u32 h) {
        const char* V="#version 330\nlayout(location=0)in vec3 P;layout(location=1)in vec2 U;uniform mat4 Pj,M;out vec2 vU;void main(){gl_Position=Pj*M*vec4(P,1);vU=U;}";
        const char* F="#version 330\nin vec2 vU;out vec4 C;uniform vec4 Cl;uniform int T;uniform sampler2D Tx;void main(){if(T==0)C=Cl;else if(T==1)C=texture(Tx,vU)*Cl;else C=vec4(Cl.rgb,Cl.a*texture(Tx,vU).r);}";
        g_UI.s=g_Device->CreateShader(V,F); g_UI.p=Math::Mat4::Ortho(0,(f32)w,(f32)h,0,-1,1);
        f32 v[]={0,0,0,0,0, 1,0,0,1,0, 1,1,0,1,1, 0,1,0,0,1}; u32 i[]={0,1,2,2,3,0};
        g_UI.v=g_Device->CreateBuffer({BufferType::Vertex,BufferUsage::Dynamic,sizeof(v),v});
        g_UI.i=g_Device->CreateBuffer({BufferType::Index,BufferUsage::Static,sizeof(i),i});
    }}
    void Backend::Init() { g_Device=new GLRD(); g_Device->Init(); }
    void Backend::Clear(f32 r, f32 g, f32 b) { g_Device->Clear(r,g,b); }
}

// Assets
namespace Assets {
    std::vector<Texture> g_Tex; std::vector<Font> g_Fnt; std::vector<Material> g_Mat;
    void Loader::Init() {}
    Texture* Loader::LoadTexture(const char* p, bool px) {
        int w,h,c; u8* d=stbi_load(p,&w,&h,&c,4); if(!d)return 0;
        g_Tex.push_back({Graphics::g_Device->CreateTexture({(u32)w,(u32)h,Graphics::TextureFormat::RGBA8,px?Graphics::TextureFilter::Nearest:Graphics::TextureFilter::Linear,d}),(u32)w,(u32)h});
        stbi_image_free(d); return &g_Tex.back();
    }
    Font* Loader::LoadFont(const char* p, f32 sz) {
        FILE* f=fopen(p,"rb"); if(!f)return 0; fseek(f,0,SEEK_END); long s=ftell(f); rewind(f); u8* b=(u8*)malloc(s); fread(b,1,s,f); fclose(f);
        u8* map=(u8*)malloc(512*512); stbtt_bakedchar* cd=(stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar)*96);
        stbtt_BakeFontBitmap(b,0,sz,map,512,512,32,96,cd);
        g_Fnt.push_back({Graphics::g_Device->CreateTexture({512,512,Graphics::TextureFormat::R8,Graphics::TextureFilter::Linear,map}),512,512,sz,cd});
        free(b); free(map); return &g_Fnt.back();
    }
    Material* Loader::CreateSpriteMaterial(Texture* t) { return 0; }
}

// State & Engine
void StateManager::SwitchState(IState* s) { next=s; }
void StateManager::Update(f32 dt) {
    if(next) { if(curr){curr->OnExit();delete curr;} Engine::Get()->w.Clear(); EventBus::ResetState(); curr=next; next=0; curr->OnEnter(); }
    if(curr) curr->OnUpdate(dt);
}
void StateManager::Shutdown() { if(curr){curr->OnExit();delete curr;} }
Engine::Context* Engine::Get() { static Context c; return &c; }
void Engine::Init(const char* t, u32 w, u32 h) {
    Context* c=Get(); c->run=true; Window::System::Init(w,h,t); Graphics::Backend::Init(); Graphics::UI::Init(w,h); Input::Manager::Init(); Assets::Loader::Init(); EventBus::Init();
    c->w.RegisterComponent<TransformComponent>(COMP_TRANSFORM); c->w.RegisterComponent<SpriteComponent>(COMP_SPRITE); c->w.RegisterComponent<PlayerControllerComponent>(COMP_CONTROLLER);
    c->w.RegisterComponent<VelocityComponent>(COMP_VELOCITY); c->w.RegisterComponent<LifetimeComponent>(COMP_LIFETIME); c->w.RegisterComponent<UI::RectComponent>(COMP_UI_RECT);
    c->w.RegisterComponent<UI::ButtonComponent>(COMP_UI_BUTTON); c->w.RegisterComponent<UI::TextComponent>(COMP_UI_TEXT);
}
void Engine::Shutdown() { Get()->sm.Shutdown(); EventBus::Shutdown(); Window::System::Shutdown(); }
void Engine::Quit() { Get()->run=false; }
bool Engine::IsRunning() { return Get()->run; }
}
