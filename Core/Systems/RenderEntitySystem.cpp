#include "RenderEntitySystem.hpp"
#include "../../Scene/SceneManager.hpp"
#include "../../Assets/Loader.hpp"
#include "../../Platform/Window.hpp"
#include <gl/GL.h>
#include <algorithm>

namespace {
    Titan::Scene::SceneManager* g_sceneManager = nullptr;
    void EnsureTextureLoaded(unsigned int &tex, const std::string &path, int &w, int &h) {
        if (tex == 0 && !path.empty()) {
            auto t = Titan::Assets::Loader::LoadTexture(path.c_str());
            tex = t.id;
            w = t.width;
            h = t.height;
        }
    }
}

namespace Titan::Core::Systems {

void RenderEntitySystem::Update(Titan::Core::FrameContext& ctx) {
    if (!g_sceneManager) return;
    Titan::Scene::Scene* s = g_sceneManager->GetActiveScene();
    if (!s || !s->world) return;

    HWND hwnd = Titan::Platform::Window::GetHWND();
    RECT rc;
    int w = 1280, h = 720;
    if (hwnd && GetClientRect(hwnd, &rc)) {
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    for (auto const& kv : s->world->sprites) {
        auto e = kv.first;
        auto sprite = kv.second;
        auto it = s->world->transforms.find(e);
        if (it == s->world->transforms.end()) continue;
        auto &t = it->second;
        unsigned int tex = sprite.texId;
        int tw = sprite.w;
        int th = sprite.h;
        EnsureTextureLoaded(tex, sprite.path, tw, th);

        float hw = (tw > 0) ? tw * 0.5f * t.sx : 32.0f * 0.5f * t.sx;
        float hh = (th > 0) ? th * 0.5f * t.sy : 32.0f * 0.5f * t.sy;
        float cx = t.x;
        float cy = t.y;

        if (tex) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex);
            glColor4f(1,1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0); glVertex2f(cx - hw, cy - hh);
            glTexCoord2f(1,0); glVertex2f(cx + hw, cy - hh);
            glTexCoord2f(1,1); glVertex2f(cx + hw, cy + hh);
            glTexCoord2f(0,1); glVertex2f(cx - hw, cy + hh);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        } else {
            glColor3f(0.6f, 0.6f, 0.6f);
            glBegin(GL_QUADS);
            glVertex2f(cx - hw, cy - hh);
            glVertex2f(cx + hw, cy - hh);
            glVertex2f(cx + hw, cy + hh);
            glVertex2f(cx - hw, cy + hh);
            glEnd();
        }
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

}
