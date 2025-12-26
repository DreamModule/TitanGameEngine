#ifndef TITAN_UI_HPP
#define TITAN_UI_HPP

#include "Titan_ECS.hpp"
#include "Titan_Input.hpp"
#include "Titan_Graphics.hpp"
#include "stb_truetype.h"

namespace Titan::UI {

struct InteractionSystem {
    static void Update(ECS::World& w) {
        auto* btns = w.GetPool<ButtonComponent>(COMP_UI_BUTTON);
        auto* rects = w.GetPool<RectComponent>(COMP_UI_RECT);
        auto* trans = w.GetPool<TransformComponent>(COMP_TRANSFORM);
        if(!btns || !trans) return;

        Math::Vec2 m = Input::Manager::GetMousePosition();
        bool click = Input::Manager::GetActionDown("Click");

        for(u32 i=0; i<btns->data.size(); ++i) {
            ECS::EntityID id = btns->dense[i];
            ButtonComponent& b = btns->data[i];
            if(TransformComponent* t = trans->Get(id)) {
                RectComponent* r = rects?rects->Get(id):nullptr;
                f32 w = r?r->size.x:1.0f; f32 h = r?r->size.y:1.0f;
                bool hit = (m.x >= t->position.x && m.x <= t->position.x+w && m.y >= t->position.y && m.y <= t->position.y+h);
                
                b.isHovered = hit;
                if(hit && click) {
                    b.isPressed = true;
                    if(b.onClick!=0) EventBus::Emit(b.onClick, {id});
                } else if(!click) b.isPressed = false;

                if(r) {
                    if(b.isPressed) r->color = r->prsCol;
                    else if(b.isHovered) r->color = r->hovCol;
                    else r->color = r->defCol;
                }
            }
        }
    }
};

struct RenderCommand { i32 z; u32 type; ECS::EntityID e; };

struct RenderSystem {
    static void Update(ECS::World& w) {
        auto* rects = w.GetPool<RectComponent>(COMP_UI_RECT);
        auto* texts = w.GetPool<TextComponent>(COMP_UI_TEXT);
        auto* trans = w.GetPool<TransformComponent>(COMP_TRANSFORM);
        if(!trans) return;

        std::vector<RenderCommand> cmds;
        if(rects) for(u32 i=0;i<rects->data.size();++i) cmds.push_back({rects->data[i].zIndex,0,rects->dense[i]});
        if(texts) for(u32 i=0;i<texts->data.size();++i) cmds.push_back({texts->data[i].zIndex,1,texts->dense[i]});
        std::sort(cmds.begin(), cmds.end(), [](const RenderCommand& a, const RenderCommand& b){return a.z<b.z;});

        Graphics::g_Device->BindPipeline(Graphics::UI::g_UI.s);
        Graphics::g_Device->SetUniformMat4(Graphics::UI::g_UI.s, "P", Graphics::UI::g_UI.p);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_DEPTH_TEST);

        for(const auto& c : cmds) {
            TransformComponent* t = trans->Get(c.e);
            if(!t) continue;
            if(c.type==0) {
                RectComponent* r = rects->Get(c.e);
                Math::Mat4 m = Math::Mat4::Translate(t->position);
                m = Math::Mat4::Mul(m, Math::Mat4::Scale({r->size.x*t->scale.x, r->size.y*t->scale.y, 1}));
                Graphics::g_Device->SetUniformMat4(Graphics::UI::g_UI.s, "M", m);
                Graphics::g_Device->SetUniformVec4(Graphics::UI::g_UI.s, "C", r->color);
                Graphics::g_Device->SetUniformInt(Graphics::UI::g_UI.s, "T", 0);
                Graphics::g_Device->BindVertexBuffer(Graphics::UI::g_UI.v);
                Graphics::g_Device->BindIndexBuffer(Graphics::UI::g_UI.i);
                Graphics::g_Device->DrawIndexed(6,0);
            } else if(c.type==1) {
                TextComponent* txt = texts->Get(c.e);
                if(!txt->font) continue;
                Graphics::g_Device->BindTexture(txt->font->atlas, 0);
                Graphics::g_Device->SetUniformInt(Graphics::UI::g_UI.s, "T", 2);
                Graphics::g_Device->SetUniformVec4(Graphics::UI::g_UI.s, "C", txt->color);
                f32 x = t->position.x, y = t->position.y + txt->font->size;
                const char* s = txt->text;
                while(*s) {
                    if(*s>=32 && *s<128) {
                        stbtt_aligned_quad q;
                        stbtt_GetBakedQuad(txt->font->glyphs, 512, 512, *s-32, &x, &y, &q, 1);
                        f32 v[]={q.x0,q.y0,0,q.s0,q.t0, q.x1,q.y0,0,q.s1,q.t0, q.x1,q.y1,0,q.s1,q.t1, q.x0,q.y1,0,q.s0,q.t1};
                        Graphics::g_Device->UpdateBuffer(Graphics::UI::g_UI.v, 0, sizeof(v), v);
                        Graphics::g_Device->DrawIndexed(6,0);
                    }
                    ++s;
                }
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
};
}
#endif
