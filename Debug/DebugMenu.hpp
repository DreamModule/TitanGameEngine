#ifndef TITAN_DEBUG_MENU_HPP
#define TITAN_DEBUG_MENU_HPP

#include <windows.h>
#include <gl/GL.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <functional>
#include <deque>

namespace Titan {
namespace Debug {

struct GraphData {
    std::string name;
    std::deque<float> values;
    float minVal = 0;
    float maxVal = 100;
    float r = 0, g = 1, b = 0;
    int maxSamples = 120;
    
    void Push(float v) {
        values.push_back(v);
        while (values.size() > (size_t)maxSamples) values.pop_front();
    }
};

struct DebugVar {
    std::string name;
    std::string value;
    float r = 1, g = 1, b = 1;
};

struct Hitbox {
    float x, y, z;
    float width, height, depth;
    float r, g, b, a;
    bool active;
};

class DebugMenu {
public:
    bool showMenu = false;
    bool showStats = true;
    bool showGraphs = true;
    bool showHitboxes = false;
    bool showNetInfo = true;
    bool showPlayerInfo = true;
    bool showConsole = false;
    bool wireframeMode = false;
    bool showColliders = false;
    bool showVelocity = false;
    bool showServerPos = false;
    bool showInterpBuffer = false;
    bool pauseGame = false;
    
    int screenWidth = 1280;
    int screenHeight = 720;
    
    void Toggle() { showMenu = !showMenu; }
    void Open() { showMenu = true; }
    void Close() { showMenu = false; }
    
    void SetVar(const char* name, const char* fmt, ...) {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        
        for (auto& v : m_vars) {
            if (v.name == name) { v.value = buf; return; }
        }
        m_vars.push_back({name, buf, 1, 1, 1});
    }
    
    void SetVarColor(const char* name, float r, float g, float b) {
        for (auto& v : m_vars) {
            if (v.name == name) { v.r = r; v.g = g; v.b = b; return; }
        }
    }
    
    void AddGraph(const char* name, float min, float max, float r, float g, float b) {
        for (auto& gr : m_graphs) {
            if (gr.name == name) return;
        }
        GraphData gd;
        gd.name = name;
        gd.minVal = min;
        gd.maxVal = max;
        gd.r = r;
        gd.g = g;
        gd.b = b;
        m_graphs.push_back(gd);
    }
    
    void PushGraph(const char* name, float value) {
        for (auto& gr : m_graphs) {
            if (gr.name == name) { gr.Push(value); return; }
        }
    }
    
    void AddHitbox(float x, float y, float z, float w, float h, float d, float r, float g, float b, float a = 1.0f) {
        m_hitboxes.push_back({x, y, z, w, h, d, r, g, b, a, true});
    }
    
    void ClearHitboxes() { m_hitboxes.clear(); }
    
    void Log(const char* fmt, ...) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        
        m_console.push_back(buf);
        while (m_console.size() > 20) m_console.pop_front();
    }
    
    void Update(float dt, bool* keys, bool* keysDown, int mouseX, int mouseY, bool mouseClick) {
        m_mouseX = mouseX;
        m_mouseY = mouseY;
        m_mouseClick = mouseClick;
        
        if (keysDown[0x2D]) showMenu = !showMenu;
        if (keysDown[VK_F1]) showStats = !showStats;
        if (keysDown[VK_F2]) showGraphs = !showGraphs;
        if (keysDown[VK_F3]) showHitboxes = !showHitboxes;
        if (keysDown[VK_F4]) wireframeMode = !wireframeMode;
        if (keysDown[VK_F5]) showConsole = !showConsole;
        if (keysDown[VK_F6]) showColliders = !showColliders;
        if (keysDown[VK_F7]) showServerPos = !showServerPos;
        if (keysDown[VK_F8]) pauseGame = !pauseGame;
        if (keysDown[VK_F9]) showVelocity = !showVelocity;
        
        if (keysDown[0xC0]) showMenu = !showMenu;
    }
    
    void DrawHitboxes3D() {
        if (!showHitboxes) return;
        
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0f);
        
        for (auto& hb : m_hitboxes) {
            if (!hb.active) continue;
            
            float x = hb.x, y = hb.y, z = hb.z;
            float hw = hb.width * 0.5f;
            float hh = hb.height * 0.5f;
            float hd = hb.depth * 0.5f;
            
            if (hb.a < 1.0f) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(hb.r, hb.g, hb.b, hb.a * 0.3f);
                glBegin(GL_QUADS);
                glVertex3f(x-hw, y-hh, z+hd); glVertex3f(x+hw, y-hh, z+hd); glVertex3f(x+hw, y+hh, z+hd); glVertex3f(x-hw, y+hh, z+hd);
                glVertex3f(x+hw, y-hh, z-hd); glVertex3f(x-hw, y-hh, z-hd); glVertex3f(x-hw, y+hh, z-hd); glVertex3f(x+hw, y+hh, z-hd);
                glVertex3f(x-hw, y+hh, z+hd); glVertex3f(x+hw, y+hh, z+hd); glVertex3f(x+hw, y+hh, z-hd); glVertex3f(x-hw, y+hh, z-hd);
                glVertex3f(x-hw, y-hh, z-hd); glVertex3f(x+hw, y-hh, z-hd); glVertex3f(x+hw, y-hh, z+hd); glVertex3f(x-hw, y-hh, z+hd);
                glVertex3f(x+hw, y-hh, z+hd); glVertex3f(x+hw, y-hh, z-hd); glVertex3f(x+hw, y+hh, z-hd); glVertex3f(x+hw, y+hh, z+hd);
                glVertex3f(x-hw, y-hh, z-hd); glVertex3f(x-hw, y-hh, z+hd); glVertex3f(x-hw, y+hh, z+hd); glVertex3f(x-hw, y+hh, z-hd);
                glEnd();
                glDisable(GL_BLEND);
            }
            
            glColor4f(hb.r, hb.g, hb.b, 1.0f);
            glBegin(GL_LINES);
            glVertex3f(x-hw, y-hh, z-hd); glVertex3f(x+hw, y-hh, z-hd);
            glVertex3f(x+hw, y-hh, z-hd); glVertex3f(x+hw, y+hh, z-hd);
            glVertex3f(x+hw, y+hh, z-hd); glVertex3f(x-hw, y+hh, z-hd);
            glVertex3f(x-hw, y+hh, z-hd); glVertex3f(x-hw, y-hh, z-hd);
            
            glVertex3f(x-hw, y-hh, z+hd); glVertex3f(x+hw, y-hh, z+hd);
            glVertex3f(x+hw, y-hh, z+hd); glVertex3f(x+hw, y+hh, z+hd);
            glVertex3f(x+hw, y+hh, z+hd); glVertex3f(x-hw, y+hh, z+hd);
            glVertex3f(x-hw, y+hh, z+hd); glVertex3f(x-hw, y-hh, z+hd);
            
            glVertex3f(x-hw, y-hh, z-hd); glVertex3f(x-hw, y-hh, z+hd);
            glVertex3f(x+hw, y-hh, z-hd); glVertex3f(x+hw, y-hh, z+hd);
            glVertex3f(x+hw, y+hh, z-hd); glVertex3f(x+hw, y+hh, z+hd);
            glVertex3f(x-hw, y+hh, z-hd); glVertex3f(x-hw, y+hh, z+hd);
            glEnd();
        }
        
        glEnable(GL_LIGHTING);
    }
    
    void DrawVelocityVector(float x, float y, float z, float vx, float vy, float vz, float scale = 0.2f) {
        if (!showVelocity) return;
        
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        glColor3f(1, 1, 0);
        glBegin(GL_LINES);
        glVertex3f(x, y, z);
        glVertex3f(x + vx * scale, y + vy * scale, z + vz * scale);
        glEnd();
        glEnable(GL_LIGHTING);
    }
    
    void DrawServerPosition(float clientX, float clientY, float clientZ, float serverX, float serverY, float serverZ) {
        if (!showServerPos) return;
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        glColor3f(1, 0, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        float r = 0.3f;
        for (int i = 0; i < 16; i++) {
            float a = i * 3.14159f * 2.0f / 16.0f;
            glVertex3f(serverX + cosf(a) * r, serverY, serverZ + sinf(a) * r);
        }
        glEnd();
        
        glColor4f(1, 0.5f, 0, 0.5f);
        glBegin(GL_LINES);
        glVertex3f(clientX, clientY, clientZ);
        glVertex3f(serverX, serverY, serverZ);
        glEnd();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
    
    void Draw2D() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        if (showStats) DrawStats();
        if (showGraphs) DrawGraphs();
        if (showMenu) DrawMenu();
        if (showConsole) DrawConsole();
        DrawHotkeys();
        
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

private:
    void DrawRoundedRect(float x, float y, float w, float h, float radius, float r, float g, float b, float a) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(r, g, b, a);
        
        glBegin(GL_QUADS);
        glVertex2f(x + radius, y);
        glVertex2f(x + w - radius, y);
        glVertex2f(x + w - radius, y + h);
        glVertex2f(x + radius, y + h);
        
        glVertex2f(x, y + radius);
        glVertex2f(x + radius, y + radius);
        glVertex2f(x + radius, y + h - radius);
        glVertex2f(x, y + h - radius);
        
        glVertex2f(x + w - radius, y + radius);
        glVertex2f(x + w, y + radius);
        glVertex2f(x + w, y + h - radius);
        glVertex2f(x + w - radius, y + h - radius);
        glEnd();
        
        int segs = 8;
        float corners[4][2] = {
            {x + radius, y + radius},
            {x + w - radius, y + radius},
            {x + w - radius, y + h - radius},
            {x + radius, y + h - radius}
        };
        float angles[4] = {3.14159f, 3.14159f * 1.5f, 0, 3.14159f * 0.5f};
        
        for (int c = 0; c < 4; c++) {
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(corners[c][0], corners[c][1]);
            for (int i = 0; i <= segs; i++) {
                float a = angles[c] + (3.14159f * 0.5f) * i / segs;
                glVertex2f(corners[c][0] + cosf(a) * radius, corners[c][1] + sinf(a) * radius);
            }
            glEnd();
        }
        
        glDisable(GL_BLEND);
    }
    
    void DrawGlow(float x, float y, float w, float h, float r, float g, float b) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        float glowSize = 20;
        for (int i = 0; i < 10; i++) {
            float t = i / 10.0f;
            float alpha = 0.05f * (1.0f - t);
            float offset = glowSize * t;
            glColor4f(r, g, b, alpha);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x - offset, y - offset);
            glVertex2f(x + w + offset, y - offset);
            glVertex2f(x + w + offset, y + h + offset);
            glVertex2f(x - offset, y + h + offset);
            glEnd();
        }
        
        glDisable(GL_BLEND);
    }
    
    void DrawStats() {
        float panelW = 240;
        float lineH = 18;
        float panelH = (m_vars.size() + 2) * lineH + 25;
        float x = 15, y = 15;
        
        DrawGlow(x, y, panelW, panelH, 0, 0.8f, 1);
        DrawRoundedRect(x, y, panelW, panelH, 8, 0.02f, 0.02f, 0.05f, 0.92f);
        
        glColor3f(0, 0.9f, 1);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 8, y); glVertex2f(x + panelW - 8, y);
        glVertex2f(x + panelW, y + 8); glVertex2f(x + panelW, y + panelH - 8);
        glVertex2f(x + panelW - 8, y + panelH); glVertex2f(x + 8, y + panelH);
        glVertex2f(x, y + panelH - 8); glVertex2f(x, y + 8);
        glEnd();
        
        float ty = y + 22;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0, 0.9f, 1, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x + 10, ty); glVertex2f(x + panelW - 10, ty);
        glVertex2f(x + panelW - 10, ty + 1); glVertex2f(x + 10, ty + 1);
        glEnd();
        glDisable(GL_BLEND);
        
        ty += 8;
        
        for (size_t i = 0; i < m_vars.size(); i++) {
            glColor3f(0.4f, 0.4f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(x + 12, ty + 3);
            glVertex2f(x + 16, ty + 3);
            glVertex2f(x + 16, ty + 12);
            glVertex2f(x + 12, ty + 12);
            glEnd();
            
            glColor3f(m_vars[i].r, m_vars[i].g, m_vars[i].b);
            glPointSize(4);
            glBegin(GL_POINTS);
            glVertex2f(x + 14, ty + 7);
            glEnd();
            
            ty += lineH;
        }
    }
    
    void DrawGraphs() {
        if (m_graphs.empty()) return;
        
        float graphW = 180;
        float graphH = 50;
        float startX = screenWidth - graphW - 20;
        float startY = 15;
        
        for (size_t i = 0; i < m_graphs.size(); i++) {
            auto& gr = m_graphs[i];
            float y = startY + i * (graphH + 25);
            
            DrawRoundedRect(startX - 5, y - 5, graphW + 10, graphH + 20, 6, 0.02f, 0.02f, 0.05f, 0.9f);
            
            glColor3f(0.15f, 0.15f, 0.2f);
            glBegin(GL_QUADS);
            glVertex2f(startX, y + 10); glVertex2f(startX + graphW, y + 10);
            glVertex2f(startX + graphW, y + 10 + graphH); glVertex2f(startX, y + 10 + graphH);
            glEnd();
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(gr.r, gr.g, gr.b, 0.1f);
            glBegin(GL_LINES);
            glVertex2f(startX, y + 10 + graphH * 0.5f);
            glVertex2f(startX + graphW, y + 10 + graphH * 0.5f);
            glVertex2f(startX, y + 10 + graphH * 0.25f);
            glVertex2f(startX + graphW, y + 10 + graphH * 0.25f);
            glVertex2f(startX, y + 10 + graphH * 0.75f);
            glVertex2f(startX + graphW, y + 10 + graphH * 0.75f);
            glEnd();
            glDisable(GL_BLEND);
            
            if (!gr.values.empty()) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glColor4f(gr.r, gr.g, gr.b, 0.2f);
                glBegin(GL_QUAD_STRIP);
                for (size_t j = 0; j < gr.values.size(); j++) {
                    float px = startX + (float)j / gr.maxSamples * graphW;
                    float normalized = (gr.values[j] - gr.minVal) / (gr.maxVal - gr.minVal);
                    if (normalized < 0) normalized = 0;
                    if (normalized > 1) normalized = 1;
                    float py = y + 10 + graphH - normalized * graphH;
                    glVertex2f(px, y + 10 + graphH);
                    glVertex2f(px, py);
                }
                glEnd();
                glDisable(GL_BLEND);
                
                glColor3f(gr.r, gr.g, gr.b);
                glLineWidth(2);
                glBegin(GL_LINE_STRIP);
                for (size_t j = 0; j < gr.values.size(); j++) {
                    float px = startX + (float)j / gr.maxSamples * graphW;
                    float normalized = (gr.values[j] - gr.minVal) / (gr.maxVal - gr.minVal);
                    if (normalized < 0) normalized = 0;
                    if (normalized > 1) normalized = 1;
                    float py = y + 10 + graphH - normalized * graphH;
                    glVertex2f(px, py);
                }
                glEnd();
                
                float lastVal = gr.values.back();
                float normalized = (lastVal - gr.minVal) / (gr.maxVal - gr.minVal);
                if (normalized < 0) normalized = 0;
                if (normalized > 1) normalized = 1;
                float dotY = y + 10 + graphH - normalized * graphH;
                
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                for (int g = 0; g < 5; g++) {
                    float ga = 0.3f * (1.0f - g / 5.0f);
                    float gr2 = 4 + g * 2;
                    glColor4f(gr.r, gr.g, gr.b, ga);
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex2f(startX + graphW - 2, dotY);
                    for (int s = 0; s <= 12; s++) {
                        float a = s * 3.14159f * 2.0f / 12;
                        glVertex2f(startX + graphW - 2 + cosf(a) * gr2, dotY + sinf(a) * gr2);
                    }
                    glEnd();
                }
                glDisable(GL_BLEND);
                
                glColor3f(1, 1, 1);
                glPointSize(6);
                glBegin(GL_POINTS);
                glVertex2f(startX + graphW - 2, dotY);
                glEnd();
            }
            
            glColor3f(gr.r * 0.7f, gr.g * 0.7f, gr.b * 0.7f);
            glLineWidth(1);
            glBegin(GL_LINE_LOOP);
            glVertex2f(startX, y + 10); glVertex2f(startX + graphW, y + 10);
            glVertex2f(startX + graphW, y + 10 + graphH); glVertex2f(startX, y + 10 + graphH);
            glEnd();
        }
    }
    
    void DrawMenu() {
        float panelW = 320;
        float panelH = 450;
        float x = (screenWidth - panelW) * 0.5f;
        float y = (screenHeight - panelH) * 0.5f;
        
        DrawGlow(x, y, panelW, panelH, 0.5f, 0, 1);
        DrawRoundedRect(x, y, panelW, panelH, 12, 0.03f, 0.02f, 0.08f, 0.97f);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.5f, 0, 1, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + panelW, y);
        glVertex2f(x + panelW, y + 40); glVertex2f(x, y + 40);
        glEnd();
        glDisable(GL_BLEND);
        
        glColor3f(0.7f, 0.3f, 1);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 12, y); glVertex2f(x + panelW - 12, y);
        glVertex2f(x + panelW, y + 12); glVertex2f(x + panelW, y + panelH - 12);
        glVertex2f(x + panelW - 12, y + panelH); glVertex2f(x + 12, y + panelH);
        glVertex2f(x, y + panelH - 12); glVertex2f(x, y + 12);
        glEnd();
        
        struct MenuItem {
            const char* name;
            const char* key;
            bool* value;
        };
        
        MenuItem items[] = {
            {"Stats Overlay", "F1", &showStats},
            {"Graphs", "F2", &showGraphs},
            {"Hitboxes", "F3", &showHitboxes},
            {"Wireframe", "F4", &wireframeMode},
            {"Console", "F5", &showConsole},
            {"Colliders", "F6", &showColliders},
            {"Server Position", "F7", &showServerPos},
            {"Pause Simulation", "F8", &pauseGame},
            {"Velocity Vectors", "F9", &showVelocity},
        };
        int itemCount = sizeof(items) / sizeof(items[0]);
        
        float itemY = y + 55;
        float itemH = 36;
        
        for (int i = 0; i < itemCount; i++) {
            float ix = x + 15;
            float iy = itemY + i * itemH;
            float iw = panelW - 30;
            float ih = itemH - 4;
            
            bool hover = m_mouseX >= ix && m_mouseX <= ix + iw && m_mouseY >= iy && m_mouseY <= iy + ih;
            
            if (hover) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.5f, 0.3f, 1, 0.15f);
                glBegin(GL_QUADS);
                glVertex2f(ix, iy); glVertex2f(ix + iw, iy);
                glVertex2f(ix + iw, iy + ih); glVertex2f(ix, iy + ih);
                glEnd();
                glDisable(GL_BLEND);
            }
            
            float keyW = 35;
            float keyX = x + panelW - 55;
            float keyY = iy + 6;
            float keyH = ih - 12;
            
            if (*items[i].value) {
                DrawRoundedRect(keyX - 2, keyY - 2, keyW + 4, keyH + 4, 4, 0.2f, 0.8f, 0.4f, 0.3f);
                glColor3f(0.3f, 1, 0.5f);
            } else {
                DrawRoundedRect(keyX - 2, keyY - 2, keyW + 4, keyH + 4, 4, 0.2f, 0.2f, 0.25f, 0.5f);
                glColor3f(0.4f, 0.4f, 0.5f);
            }
            
            glLineWidth(1);
            glBegin(GL_LINE_LOOP);
            glVertex2f(keyX, keyY); glVertex2f(keyX + keyW, keyY);
            glVertex2f(keyX + keyW, keyY + keyH); glVertex2f(keyX, keyY + keyH);
            glEnd();
            
            float checkX = ix + 8;
            float checkY = iy + ih * 0.5f - 6;
            float checkS = 12;
            
            if (*items[i].value) {
                glColor3f(0.3f, 1, 0.5f);
                glBegin(GL_QUADS);
                glVertex2f(checkX + 2, checkY + 2);
                glVertex2f(checkX + checkS - 2, checkY + 2);
                glVertex2f(checkX + checkS - 2, checkY + checkS - 2);
                glVertex2f(checkX + 2, checkY + checkS - 2);
                glEnd();
            }
            
            glColor3f(0.5f, 0.5f, 0.6f);
            glLineWidth(1);
            glBegin(GL_LINE_LOOP);
            glVertex2f(checkX, checkY); glVertex2f(checkX + checkS, checkY);
            glVertex2f(checkX + checkS, checkY + checkS); glVertex2f(checkX, checkY + checkS);
            glEnd();
        }
        
        glColor3f(0.4f, 0.3f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(x + 15, y + 40);
        glVertex2f(x + panelW - 15, y + 40);
        glEnd();
    }
    
    void DrawConsole() {
        float panelW = 550;
        float panelH = 200;
        float x = 15;
        float y = screenHeight - panelH - 45;
        
        DrawRoundedRect(x, y, panelW, panelH, 8, 0.01f, 0.01f, 0.02f, 0.95f);
        
        glColor3f(0.2f, 0.8f, 0.3f);
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 8, y); glVertex2f(x + panelW - 8, y);
        glVertex2f(x + panelW, y + 8); glVertex2f(x + panelW, y + panelH - 8);
        glVertex2f(x + panelW - 8, y + panelH); glVertex2f(x + 8, y + panelH);
        glVertex2f(x, y + panelH - 8); glVertex2f(x, y + 8);
        glEnd();
        
        float lineY = y + 15;
        for (size_t i = 0; i < m_console.size(); i++) {
            glColor3f(0.2f, 0.7f, 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(x + 10, lineY); glVertex2f(x + 14, lineY);
            glVertex2f(x + 14, lineY + 3); glVertex2f(x + 10, lineY + 3);
            glEnd();
            lineY += 10;
        }
    }
    
    void DrawHotkeys() {
        float barH = 28;
        float y = screenHeight - barH - 5;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0, 0, 0, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(5, y); glVertex2f(screenWidth - 5, y);
        glVertex2f(screenWidth - 5, y + barH); glVertex2f(5, y + barH);
        glEnd();
        glDisable(GL_BLEND);
        
        struct Key {
            const char* label;
            bool active;
            float r, g, b;
        };
        
        Key keys[] = {
            {"INS", showMenu, 0.7f, 0.3f, 1},
            {"F1", showStats, 0, 0.9f, 1},
            {"F2", showGraphs, 0, 0.9f, 1},
            {"F3", showHitboxes, 0.3f, 1, 0.5f},
            {"F4", wireframeMode, 1, 0.5f, 0},
            {"F5", showConsole, 0.2f, 0.8f, 0.3f},
            {"F6", showColliders, 0.3f, 1, 0.5f},
            {"F7", showServerPos, 1, 0.3f, 0.3f},
            {"F8", pauseGame, 1, 1, 0},
            {"F9", showVelocity, 1, 1, 0}
        };
        int keyCount = sizeof(keys) / sizeof(keys[0]);
        
        float kx = 15;
        float kw = 40;
        float kh = 20;
        float ky = y + 4;
        
        for (int i = 0; i < keyCount; i++) {
            if (keys[i].active) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glColor4f(keys[i].r, keys[i].g, keys[i].b, 0.3f);
                glBegin(GL_QUADS);
                glVertex2f(kx - 2, ky - 2); glVertex2f(kx + kw + 2, ky - 2);
                glVertex2f(kx + kw + 2, ky + kh + 2); glVertex2f(kx - 2, ky + kh + 2);
                glEnd();
                glDisable(GL_BLEND);
                
                glColor3f(keys[i].r, keys[i].g, keys[i].b);
            } else {
                glColor3f(0.3f, 0.3f, 0.35f);
            }
            
            glLineWidth(keys[i].active ? 2 : 1);
            glBegin(GL_LINE_LOOP);
            glVertex2f(kx, ky); glVertex2f(kx + kw, ky);
            glVertex2f(kx + kw, ky + kh); glVertex2f(kx, ky + kh);
            glEnd();
            
            kx += kw + 8;
        }
    }
    
    std::vector<DebugVar> m_vars;
    std::vector<GraphData> m_graphs;
    std::vector<Hitbox> m_hitboxes;
    std::deque<std::string> m_console;
    
    int m_mouseX = 0, m_mouseY = 0;
    bool m_mouseClick = false;
};

}
}

#endif
