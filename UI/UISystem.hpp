#ifndef TITAN_UI_SYSTEM_HPP
#define TITAN_UI_SYSTEM_HPP

#include <windows.h>
#include <gl/GL.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace Titan {
namespace UI {

struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    static Color White() { return {1, 1, 1, 1}; }
    static Color Black() { return {0, 0, 0, 1}; }
    static Color Red() { return {1, 0, 0, 1}; }
    static Color Green() { return {0, 1, 0, 1}; }
    static Color Blue() { return {0, 0, 1, 1}; }
    static Color Yellow() { return {1, 1, 0, 1}; }
    static Color Cyan() { return {0, 1, 1, 1}; }
    static Color Gray() { return {0.5f, 0.5f, 0.5f, 1}; }
    static Color Clear() { return {0, 0, 0, 0}; }
};

struct Rect {
    float x, y, w, h;
    bool Contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

struct FontGlyph {
    uint8_t bitmap[8];
    int width;
};

class BitmapFont {
public:
    void Init() {
        for (int i = 0; i < 128; i++) {
            m_glyphs[i].width = 5;
            memset(m_glyphs[i].bitmap, 0, 8);
        }
        
        const uint8_t font_A[] = {0x1C, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00};
        const uint8_t font_B[] = {0x3C, 0x22, 0x3C, 0x22, 0x22, 0x22, 0x3C, 0x00};
        const uint8_t font_C[] = {0x1C, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00};
        const uint8_t font_D[] = {0x38, 0x24, 0x22, 0x22, 0x22, 0x24, 0x38, 0x00};
        const uint8_t font_E[] = {0x3E, 0x20, 0x3C, 0x20, 0x20, 0x20, 0x3E, 0x00};
        const uint8_t font_F[] = {0x3E, 0x20, 0x3C, 0x20, 0x20, 0x20, 0x20, 0x00};
        const uint8_t font_G[] = {0x1C, 0x22, 0x20, 0x2E, 0x22, 0x22, 0x1C, 0x00};
        const uint8_t font_H[] = {0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x22, 0x00};
        const uint8_t font_I[] = {0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00};
        const uint8_t font_J[] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x24, 0x18, 0x00};
        const uint8_t font_K[] = {0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00};
        const uint8_t font_L[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00};
        const uint8_t font_M[] = {0x22, 0x36, 0x2A, 0x22, 0x22, 0x22, 0x22, 0x00};
        const uint8_t font_N[] = {0x22, 0x32, 0x2A, 0x26, 0x22, 0x22, 0x22, 0x00};
        const uint8_t font_O[] = {0x1C, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00};
        const uint8_t font_P[] = {0x3C, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x20, 0x00};
        const uint8_t font_Q[] = {0x1C, 0x22, 0x22, 0x22, 0x2A, 0x24, 0x1A, 0x00};
        const uint8_t font_R[] = {0x3C, 0x22, 0x22, 0x3C, 0x28, 0x24, 0x22, 0x00};
        const uint8_t font_S[] = {0x1C, 0x22, 0x10, 0x08, 0x04, 0x22, 0x1C, 0x00};
        const uint8_t font_T[] = {0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00};
        const uint8_t font_U[] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00};
        const uint8_t font_V[] = {0x22, 0x22, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00};
        const uint8_t font_W[] = {0x22, 0x22, 0x22, 0x2A, 0x2A, 0x36, 0x22, 0x00};
        const uint8_t font_X[] = {0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00};
        const uint8_t font_Y[] = {0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00};
        const uint8_t font_Z[] = {0x3E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3E, 0x00};
        
        const uint8_t font_0[] = {0x1C, 0x22, 0x26, 0x2A, 0x32, 0x22, 0x1C, 0x00};
        const uint8_t font_1[] = {0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00};
        const uint8_t font_2[] = {0x1C, 0x22, 0x02, 0x0C, 0x10, 0x20, 0x3E, 0x00};
        const uint8_t font_3[] = {0x1C, 0x22, 0x02, 0x0C, 0x02, 0x22, 0x1C, 0x00};
        const uint8_t font_4[] = {0x04, 0x0C, 0x14, 0x24, 0x3E, 0x04, 0x04, 0x00};
        const uint8_t font_5[] = {0x3E, 0x20, 0x3C, 0x02, 0x02, 0x22, 0x1C, 0x00};
        const uint8_t font_6[] = {0x1C, 0x20, 0x3C, 0x22, 0x22, 0x22, 0x1C, 0x00};
        const uint8_t font_7[] = {0x3E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00};
        const uint8_t font_8[] = {0x1C, 0x22, 0x22, 0x1C, 0x22, 0x22, 0x1C, 0x00};
        const uint8_t font_9[] = {0x1C, 0x22, 0x22, 0x1E, 0x02, 0x02, 0x1C, 0x00};
        
        const uint8_t font_space[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t font_dot[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00};
        const uint8_t font_comma[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x10};
        const uint8_t font_colon[] = {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00};
        const uint8_t font_minus[] = {0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00};
        const uint8_t font_plus[] = {0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00};
        const uint8_t font_slash[] = {0x02, 0x04, 0x08, 0x08, 0x10, 0x20, 0x20, 0x00};
        const uint8_t font_percent[] = {0x30, 0x32, 0x04, 0x08, 0x10, 0x26, 0x06, 0x00};
        
        SetGlyph('A', font_A); SetGlyph('B', font_B); SetGlyph('C', font_C);
        SetGlyph('D', font_D); SetGlyph('E', font_E); SetGlyph('F', font_F);
        SetGlyph('G', font_G); SetGlyph('H', font_H); SetGlyph('I', font_I);
        SetGlyph('J', font_J); SetGlyph('K', font_K); SetGlyph('L', font_L);
        SetGlyph('M', font_M); SetGlyph('N', font_N); SetGlyph('O', font_O);
        SetGlyph('P', font_P); SetGlyph('Q', font_Q); SetGlyph('R', font_R);
        SetGlyph('S', font_S); SetGlyph('T', font_T); SetGlyph('U', font_U);
        SetGlyph('V', font_V); SetGlyph('W', font_W); SetGlyph('X', font_X);
        SetGlyph('Y', font_Y); SetGlyph('Z', font_Z);
        
        SetGlyph('a', font_A); SetGlyph('b', font_B); SetGlyph('c', font_C);
        SetGlyph('d', font_D); SetGlyph('e', font_E); SetGlyph('f', font_F);
        SetGlyph('g', font_G); SetGlyph('h', font_H); SetGlyph('i', font_I);
        SetGlyph('j', font_J); SetGlyph('k', font_K); SetGlyph('l', font_L);
        SetGlyph('m', font_M); SetGlyph('n', font_N); SetGlyph('o', font_O);
        SetGlyph('p', font_P); SetGlyph('q', font_Q); SetGlyph('r', font_R);
        SetGlyph('s', font_S); SetGlyph('t', font_T); SetGlyph('u', font_U);
        SetGlyph('v', font_V); SetGlyph('w', font_W); SetGlyph('x', font_X);
        SetGlyph('y', font_Y); SetGlyph('z', font_Z);
        
        SetGlyph('0', font_0); SetGlyph('1', font_1); SetGlyph('2', font_2);
        SetGlyph('3', font_3); SetGlyph('4', font_4); SetGlyph('5', font_5);
        SetGlyph('6', font_6); SetGlyph('7', font_7); SetGlyph('8', font_8);
        SetGlyph('9', font_9);
        
        SetGlyph(' ', font_space); SetGlyph('.', font_dot);
        SetGlyph(',', font_comma); SetGlyph(':', font_colon);
        SetGlyph('-', font_minus); SetGlyph('+', font_plus);
        SetGlyph('/', font_slash); SetGlyph('%', font_percent);
        
        m_initialized = true;
    }
    
    void DrawChar(char c, float x, float y, float scale, const Color& color) {
        if (c < 0 || c >= 128) return;
        const FontGlyph& g = m_glyphs[(int)c];
        
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_QUADS);
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 6; col++) {
                if (g.bitmap[row] & (0x20 >> col)) {
                    float px = x + col * scale;
                    float py = y + row * scale;
                    glVertex2f(px, py);
                    glVertex2f(px + scale, py);
                    glVertex2f(px + scale, py + scale);
                    glVertex2f(px, py + scale);
                }
            }
        }
        glEnd();
    }
    
    void DrawText(const char* text, float x, float y, float scale, const Color& color) {
        float cx = x;
        while (*text) {
            DrawChar(*text, cx, y, scale, color);
            cx += 6 * scale;
            text++;
        }
    }
    
    float MeasureText(const char* text, float scale) {
        return strlen(text) * 6 * scale;
    }

private:
    void SetGlyph(char c, const uint8_t* data) {
        memcpy(m_glyphs[(int)c].bitmap, data, 8);
        m_glyphs[(int)c].width = 6;
    }
    
    FontGlyph m_glyphs[128];
    bool m_initialized = false;
};

enum class Anchor {
    TopLeft, TopCenter, TopRight,
    CenterLeft, Center, CenterRight,
    BottomLeft, BottomCenter, BottomRight
};

struct Widget {
    uint32_t id = 0;
    Rect bounds;
    bool visible = true;
    bool enabled = true;
    Anchor anchor = Anchor::TopLeft;
    float offsetX = 0, offsetY = 0;
};

struct ButtonWidget : Widget {
    std::string text;
    Color normalColor = {0.2f, 0.2f, 0.25f, 0.9f};
    Color hoverColor = {0.3f, 0.4f, 0.6f, 0.95f};
    Color pressedColor = {0.15f, 0.15f, 0.2f, 1.0f};
    Color textColor = Color::White();
    std::function<void()> onClick;
    bool hovered = false;
    bool pressed = false;
};

struct LabelWidget : Widget {
    std::string text;
    Color color = Color::White();
    float scale = 2.0f;
};

struct PanelWidget : Widget {
    Color backgroundColor = {0.1f, 0.1f, 0.15f, 0.9f};
    Color borderColor = {0.3f, 0.5f, 0.8f, 1.0f};
    float borderWidth = 2.0f;
    float cornerRadius = 8.0f;
};

struct ProgressBarWidget : Widget {
    float value = 0.5f;
    float minValue = 0;
    float maxValue = 1.0f;
    Color backgroundColor = {0.15f, 0.15f, 0.2f, 0.9f};
    Color fillColor = {0.2f, 0.7f, 0.3f, 1.0f};
    Color borderColor = {0.4f, 0.4f, 0.5f, 1.0f};
};

struct ImageWidget : Widget {
    uint32_t textureId = 0;
    Color tint = Color::White();
};

class UIManager {
public:
    int screenWidth = 1280;
    int screenHeight = 720;
    
    void Init() {
        m_font.Init();
        m_nextId = 1;
    }
    
    void Update(float dt, int mouseX, int mouseY, bool mouseDown, bool mouseClick) {
        m_mouseX = mouseX;
        m_mouseY = mouseY;
        m_mouseDown = mouseDown;
        m_mouseClick = mouseClick;
        
        for (auto& btn : m_buttons) {
            if (!btn.visible || !btn.enabled) continue;
            
            Rect r = GetScreenRect(btn);
            btn.hovered = r.Contains((float)mouseX, (float)mouseY);
            btn.pressed = btn.hovered && mouseDown;
            
            if (btn.hovered && mouseClick && btn.onClick) {
                btn.onClick();
            }
        }
    }
    
    void Draw() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        for (auto& panel : m_panels) {
            if (panel.visible) DrawPanel(panel);
        }
        
        for (auto& bar : m_progressBars) {
            if (bar.visible) DrawProgressBar(bar);
        }
        
        for (auto& btn : m_buttons) {
            if (btn.visible) DrawButton(btn);
        }
        
        for (auto& label : m_labels) {
            if (label.visible) DrawLabel(label);
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    uint32_t AddButton(const char* text, float x, float y, float w, float h, std::function<void()> onClick) {
        ButtonWidget btn;
        btn.id = m_nextId++;
        btn.text = text;
        btn.bounds = {x, y, w, h};
        btn.onClick = onClick;
        m_buttons.push_back(btn);
        return btn.id;
    }
    
    uint32_t AddLabel(const char* text, float x, float y, float scale = 2.0f, Color color = Color::White()) {
        LabelWidget lbl;
        lbl.id = m_nextId++;
        lbl.text = text;
        lbl.bounds = {x, y, 0, 0};
        lbl.scale = scale;
        lbl.color = color;
        m_labels.push_back(lbl);
        return lbl.id;
    }
    
    uint32_t AddPanel(float x, float y, float w, float h, Color bg = {0.1f, 0.1f, 0.15f, 0.9f}) {
        PanelWidget panel;
        panel.id = m_nextId++;
        panel.bounds = {x, y, w, h};
        panel.backgroundColor = bg;
        m_panels.push_back(panel);
        return panel.id;
    }
    
    uint32_t AddProgressBar(float x, float y, float w, float h, float value = 0.5f) {
        ProgressBarWidget bar;
        bar.id = m_nextId++;
        bar.bounds = {x, y, w, h};
        bar.value = value;
        m_progressBars.push_back(bar);
        return bar.id;
    }
    
    void SetLabelText(uint32_t id, const char* text) {
        for (auto& lbl : m_labels) {
            if (lbl.id == id) { lbl.text = text; return; }
        }
    }
    
    void SetProgressValue(uint32_t id, float value) {
        for (auto& bar : m_progressBars) {
            if (bar.id == id) { bar.value = value; return; }
        }
    }
    
    void SetWidgetVisible(uint32_t id, bool visible) {
        for (auto& btn : m_buttons) if (btn.id == id) { btn.visible = visible; return; }
        for (auto& lbl : m_labels) if (lbl.id == id) { lbl.visible = visible; return; }
        for (auto& panel : m_panels) if (panel.id == id) { panel.visible = visible; return; }
        for (auto& bar : m_progressBars) if (bar.id == id) { bar.visible = visible; return; }
    }
    
    void Clear() {
        m_buttons.clear();
        m_labels.clear();
        m_panels.clear();
        m_progressBars.clear();
    }
    
    void DrawText(const char* text, float x, float y, float scale, Color color) {
        m_font.DrawText(text, x, y, scale, color);
    }
    
    void DrawRect(float x, float y, float w, float h, Color color) {
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
    }
    
    void DrawRectOutline(float x, float y, float w, float h, Color color, float lineWidth = 1.0f) {
        glColor4f(color.r, color.g, color.b, color.a);
        glLineWidth(lineWidth);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
    }

private:
    Rect GetScreenRect(const Widget& w) {
        float x = w.bounds.x + w.offsetX;
        float y = w.bounds.y + w.offsetY;
        
        switch (w.anchor) {
            case Anchor::TopCenter: x += screenWidth * 0.5f; break;
            case Anchor::TopRight: x += screenWidth; break;
            case Anchor::CenterLeft: y += screenHeight * 0.5f; break;
            case Anchor::Center: x += screenWidth * 0.5f; y += screenHeight * 0.5f; break;
            case Anchor::CenterRight: x += screenWidth; y += screenHeight * 0.5f; break;
            case Anchor::BottomLeft: y += screenHeight; break;
            case Anchor::BottomCenter: x += screenWidth * 0.5f; y += screenHeight; break;
            case Anchor::BottomRight: x += screenWidth; y += screenHeight; break;
            default: break;
        }
        
        return {x, y, w.bounds.w, w.bounds.h};
    }
    
    void DrawPanel(PanelWidget& panel) {
        Rect r = GetScreenRect(panel);
        DrawRect(r.x, r.y, r.w, r.h, panel.backgroundColor);
        DrawRectOutline(r.x, r.y, r.w, r.h, panel.borderColor, panel.borderWidth);
    }
    
    void DrawButton(ButtonWidget& btn) {
        Rect r = GetScreenRect(btn);
        
        Color bg = btn.normalColor;
        if (btn.pressed) bg = btn.pressedColor;
        else if (btn.hovered) bg = btn.hoverColor;
        
        DrawRect(r.x, r.y, r.w, r.h, bg);
        DrawRectOutline(r.x, r.y, r.w, r.h, {1, 1, 1, 0.3f}, 1.0f);
        
        float textW = m_font.MeasureText(btn.text.c_str(), 2.0f);
        float textX = r.x + (r.w - textW) * 0.5f;
        float textY = r.y + (r.h - 16) * 0.5f;
        m_font.DrawText(btn.text.c_str(), textX, textY, 2.0f, btn.textColor);
    }
    
    void DrawLabel(LabelWidget& lbl) {
        Rect r = GetScreenRect(lbl);
        m_font.DrawText(lbl.text.c_str(), r.x, r.y, lbl.scale, lbl.color);
    }
    
    void DrawProgressBar(ProgressBarWidget& bar) {
        Rect r = GetScreenRect(bar);
        
        DrawRect(r.x, r.y, r.w, r.h, bar.backgroundColor);
        
        float t = (bar.value - bar.minValue) / (bar.maxValue - bar.minValue);
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        
        DrawRect(r.x + 2, r.y + 2, (r.w - 4) * t, r.h - 4, bar.fillColor);
        DrawRectOutline(r.x, r.y, r.w, r.h, bar.borderColor, 1.0f);
    }
    
    BitmapFont m_font;
    std::vector<ButtonWidget> m_buttons;
    std::vector<LabelWidget> m_labels;
    std::vector<PanelWidget> m_panels;
    std::vector<ProgressBarWidget> m_progressBars;
    
    uint32_t m_nextId = 1;
    int m_mouseX = 0, m_mouseY = 0;
    bool m_mouseDown = false;
    bool m_mouseClick = false;
};

class HUD {
public:
    UIManager* ui = nullptr;
    
    void Init(UIManager* uiManager) {
        ui = uiManager;
    }
    
    void DrawCrosshair(float size = 10.0f, float gap = 3.0f, Color color = Color::Green()) {
        float cx = ui->screenWidth * 0.5f;
        float cy = ui->screenHeight * 0.5f;
        
        glColor4f(color.r, color.g, color.b, color.a);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(cx - size, cy); glVertex2f(cx - gap, cy);
        glVertex2f(cx + gap, cy); glVertex2f(cx + size, cy);
        glVertex2f(cx, cy - size); glVertex2f(cx, cy - gap);
        glVertex2f(cx, cy + gap); glVertex2f(cx, cy + size);
        glEnd();
    }
    
    void DrawHealthBar(float health, float maxHealth = 100.0f, float x = 20, float y = -40) {
        float screenY = ui->screenHeight + y;
        float w = 200, h = 20;
        
        float t = health / maxHealth;
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        
        Color fillColor = {1.0f - t, t, 0, 1};
        
        ui->DrawRect(x, screenY, w, h, {0.1f, 0.1f, 0.15f, 0.8f});
        ui->DrawRect(x + 2, screenY + 2, (w - 4) * t, h - 4, fillColor);
        ui->DrawRectOutline(x, screenY, w, h, {0.4f, 0.4f, 0.5f, 1}, 1.0f);
        
        char buf[32];
        sprintf(buf, "%d", (int)health);
        ui->DrawText(buf, x + w + 10, screenY + 2, 2.0f, Color::White());
    }
    
    void DrawAmmo(int current, int reserve, float x = -150, float y = -40) {
        float screenX = ui->screenWidth + x;
        float screenY = ui->screenHeight + y;
        
        char buf[64];
        sprintf(buf, "%d / %d", current, reserve);
        ui->DrawText(buf, screenX, screenY, 2.5f, Color::White());
    }
    
    void DrawKillFeed(const char* killer, const char* victim, const char* weapon) {
        
    }
    
    void DrawMinimap(float x, float y, float size) {
        ui->DrawRect(x, y, size, size, {0.1f, 0.1f, 0.15f, 0.7f});
        ui->DrawRectOutline(x, y, size, size, {0.3f, 0.5f, 0.7f, 1}, 1.0f);
        
        float cx = x + size * 0.5f;
        float cy = y + size * 0.5f;
        glColor3f(0, 1, 0);
        glBegin(GL_TRIANGLES);
        glVertex2f(cx, cy - 5);
        glVertex2f(cx - 4, cy + 4);
        glVertex2f(cx + 4, cy + 4);
        glEnd();
    }
};

}
}

#endif


