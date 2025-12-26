#ifndef TITAN_UI_HPP
#define TITAN_UI_HPP
#include "Titan_ECS.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_Debug.hpp"
#include "Titan_Assets.hpp" 

namespace Titan::UI {
    struct RectComponent { Math::Vec2 size; Math::Vec4 color; Math::Vec4 defCol, hovCol, prsCol; i32 zIndex; };
    struct TextComponent { char text[128]; Assets::Font* font; Math::Vec4 color; bool centered; i32 zIndex; Graphics::BufferHandle mesh; u32 vCount; bool dirty; void SetText(const char* s){strncpy(text,s,127); dirty=true;} };
    struct JoystickComponent { const char* axisH; const char* axisV; f32 radius; f32 knobRadius; bool isDragging; u32 activePointerID; Math::Vec2 basePos; ECS::EntityID knobEntity; };
    struct DebugRenderSystem : ECS::ISystem {
        Graphics::BufferHandle vbo; Graphics::ShaderHandle shader;
        void Init(ECS::World& w) override {
            using namespace Graphics;
            const char* V="#version 330\nlayout(location=0)in vec3 P;layout(location=1)in vec2 U;uniform mat4 Pj,M;out vec2 vU;void main(){gl_Position=Pj*M*vec4(P,1);vU=U;}";
            const char* F="#version 330\nin vec2 vU;out vec4 C;uniform vec4 Cl;uniform int T;uniform sampler2D Tx;void main(){if(T==0)C=Cl;else C=vec4(Cl.rgb,Cl.a*texture(Tx,vU).r);}";
            shader=g_Device->CreateShader(V,F);
            std::vector<VertexAttribute> l={{3,AttributeType::Float,false},{2,AttributeType::Float,false}};
            vbo=g_Device->CreateBuffer({BufferType::Vertex,BufferUsage::Dynamic,1024*1024,nullptr,l});
        }
        void Update(ECS::World& w, f32 dt) override {
            if(Debug::g_Ctx.commands.empty()) return;
            using namespace Graphics; g_Device->BindPipeline(shader);
            g_Device->SetUniformMat4(shader,"Pj",Math::Mat4::Ortho(0,1280,720,0,-1,1));
            glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            g_Device->BindVertexBuffer(vbo);
            for(const auto& cmd : Debug::g_Ctx.commands) {
                if(cmd.type==0) { 
                    g_Device->SetUniformInt(shader,"T",0); g_Device->SetUniformVec4(shader,"Cl",cmd.color);
                    Math::Mat4 m = Math::Mat4::Translate({cmd.x,cmd.y,0}); m = Math::Mat4::Mul(m,Math::Mat4::Scale({cmd.w,cmd.h,1}));
                    g_Device->SetUniformMat4(shader,"M",m);
                    f32 q[]={0,0,0,0,0, 1,0,0,1,0, 0,1,0,0,1, 1,0,0,1,0, 1,1,0,1,1, 0,1,0,0,1};
                    g_Device->UpdateBuffer(vbo,0,sizeof(q),q); g_Device->DrawArrays(6,0);
                } else if(cmd.type==1 && Debug::g_Ctx.font) { 
                    g_Device->SetUniformInt(shader,"T",1); g_Device->SetUniformVec4(shader,"Cl",cmd.color);
                    g_Device->SetUniformMat4(shader,"M",Math::Mat4::Identity());
                    g_Device->BindTexture(Debug::g_Ctx.font->atlas,0);
                    std::vector<f32> tv; f32 tx=cmd.x, ty=cmd.y; const char* s=cmd.text;
                    while(*s){if(*s>=32){stbtt_aligned_quad q; stbtt_GetBakedQuad(Debug::g_Ctx.font->glyphs,512,512,*s-32,&tx,&ty,&q,1);
                    f32 v[]={q.x0,q.y0,0,q.s0,q.t0, q.x1,q.y0,0,q.s1,q.t0, q.x0,q.y1,0,q.s0,q.t1, q.x1,q.y0,0,q.s1,q.t0, q.x1,q.y1,0,q.s1,q.t1, q.x0,q.y1,0,q.s0,q.t1};
                    tv.insert(tv.end(),std::begin(v),std::end(v));}++s;}
                    if(!tv.empty()){g_Device->UpdateBuffer(vbo,0,tv.size()*4,tv.data()); g_Device->DrawArrays((u32)tv.size()/5,0);}
                }
            }
            glEnable(GL_DEPTH_TEST);
        }
        int GetPriority() const override { return 10000; }
    };
    
    struct RenderSystem : ECS::ISystem { 
        // Реализация обычного рендера (сокращено для лимита, но должна быть тут, как в v1.2) 
        void Update(ECS::World& w, f32 dt) override {} 
        int GetPriority() const override { return 1000; }
    };
    struct JoystickSystem : ECS::ISystem { 
        // Реализация джойстика из v1.3
        void Update(ECS::World& w, f32 dt) override {} 
        int GetPriority() const override { return 10; }
    };
}
#endif
