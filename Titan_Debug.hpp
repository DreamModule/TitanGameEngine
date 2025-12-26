#ifndef TITAN_DEBUG_HPP
#define TITAN_DEBUG_HPP
#include "Titan_Core.hpp"
#include "Titan_Input.hpp"
#include "Titan_Assets.hpp"
#include <vector>
#include <cstdio>
#include <cstdarg>

namespace Titan::Debug {
    struct DrawCmd { u32 type; f32 x, y, w, h; Math::Vec4 color; char text[64]; };
    struct Context {
        f32 x, y, startX, startY, padding;
        bool isActive;
        Assets::Font* font;
        std::vector<DrawCmd> commands;
        Math::Vec2 mouse; bool click;
    };
    static Context g_Ctx;

    static void Init() { g_Ctx.font = Assets::Loader::LoadFont("assets/arial.ttf", 16); g_Ctx.isActive = true; }
    static void Begin() {
        if(!g_Ctx.isActive) return;
        g_Ctx.commands.clear(); g_Ctx.startX=10; g_Ctx.startY=10; g_Ctx.x=10; g_Ctx.y=10; g_Ctx.padding=5;
        g_Ctx.mouse = Input::Manager::GetPointerPosition(0); g_Ctx.click = Input::Manager::GetPointerDown(0);
    }
    static void End() {}

    static bool Button(const char* label) {
        if(!g_Ctx.isActive) return false;
        f32 w=150, h=30;
        bool hover = (g_Ctx.mouse.x >= g_Ctx.x && g_Ctx.mouse.x <= g_Ctx.x+w && g_Ctx.mouse.y >= g_Ctx.y && g_Ctx.mouse.y <= g_Ctx.y+h);
        bool pressed = hover && g_Ctx.click;
        g_Ctx.commands.push_back({0, g_Ctx.x, g_Ctx.y, w, h, pressed ? Math::Vec4{0.1f,0.6f,0.1f,1} : (hover ? Math::Vec4{0.4f,0.4f,0.4f,0.9f} : Math::Vec4{0.2f,0.2f,0.2f,0.8f}), ""});
        DrawCmd txt = {1, g_Ctx.x+10, g_Ctx.y+20, 0, 0, {1,1,1,1}, ""}; strncpy(txt.text, label, 63);
        g_Ctx.commands.push_back(txt);
        g_Ctx.y += h + g_Ctx.padding;
        return pressed;
    }
    static void Text(const char* fmt, ...) {
        if(!g_Ctx.isActive) return;
        char buf[64]; va_list a; va_start(a, fmt); vsnprintf(buf, 63, fmt, a); va_end(a);
        DrawCmd txt = {1, g_Ctx.x, g_Ctx.y+16, 0, 0, {1,1,1,1}, ""}; strncpy(txt.text, buf, 63);
        g_Ctx.commands.push_back(txt);
        g_Ctx.y += 20 + g_Ctx.padding;
    }
}
#endif
