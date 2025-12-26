#ifndef GAME_FEATURES_HPP
#define GAME_FEATURES_HPP
#include "Titan_ECS.hpp"
namespace Button {
    struct Desc { const char* lbl; f32 x,y,w,h; Titan::EventID ev; Titan::Math::Vec4 col; };
    namespace GameButton {
        static Titan::ECS::EntityID Create(Titan::ECS::World& w, const Desc& d) {
            using namespace Titan;
            ECS::EntityID id = w.CreateEntity();
            w.AddComponent(id, COMP_TRANSFORM, {{d.x,d.y,0},{1,1,1},0});
            UI::RectComponent r; r.size={d.w,d.h}; r.defCol=d.col; r.hovCol={d.col.x*1.2f,d.col.y*1.2f,d.col.z*1.2f,1}; r.prsCol={d.col.x*0.8f,d.col.y*0.8f,d.col.z*0.8f,1}; r.color=d.col; r.zIndex=0;
            w.AddComponent(id, COMP_UI_RECT, r);
            w.AddComponent(id, COMP_UI_BUTTON, {false,false,d.ev});
            if(d.lbl) {
                static Assets::Font* f = Assets::Loader::LoadFont("arial.ttf", 24);
                UI::TextComponent t; t.SetText(d.lbl); t.font=f; t.color={1,1,1,1}; t.zIndex=1;
                w.AddComponent(id, COMP_UI_TEXT, t);
            }
            return id;
        }
    }
}
#endif
