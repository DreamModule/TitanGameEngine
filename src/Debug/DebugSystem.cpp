#include “DebugSystem.hpp”
#include “../Platform/Window.hpp”
#include “../Input/InputManager.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <algorithm>

namespace Titan::Debug {

namespace {
HGDIOBJ g_oldFont = nullptr;
HFONT g_font = nullptr;
GLuint g_fontBase = 0;
int g_clientW = 1280;
int g_clientH = 720;

```
Style g_style;
bool g_visible = true;

std::vector<LogEntry> g_logs;
const size_t MAX_LOGS = 1000;

std::unordered_map<std::string, ProfileData> g_profileData;
std::unordered_map<std::string, std::vector<float>> g_timerHistory;

std::unordered_map<std::string, std::vector<float>> g_metrics;
const size_t MAX_METRIC_HISTORY = 120;

std::vector<Panel*> g_panels;
Panel* g_draggedPanel = nullptr;
int g_dragOffsetX = 0;
int g_dragOffsetY = 0;

bool g_showStats = true;
bool g_showLog = false;
bool g_showProfiler = false;

Panel* g_statsPanel = nullptr;
Panel* g_logPanel = nullptr;
Panel* g_profilerPanel = nullptr;

float g_fps = 0.0f;
float g_frameTime = 0.0f;
std::chrono::high_resolution_clock::time_point g_lastFrameTime;
std::vector<float> g_fpsHistory;
std::vector<float> g_frameTimeHistory;

std::string GetTimestamp() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return std::string(buf);
}

int GetTextWidth(const char* text) {
    return (int)(strlen(text) * (g_style.fontSize * 0.6f));
}
```

}

void Init() {
HWND hwnd = (HWND)Titan::Platform::Window::GetNativeWindowHandle();
HDC hdc = (HDC)Titan::Platform::Window::GetNativeHandle();

```
RECT rc;
if (hwnd) GetClientRect(hwnd, &rc);
g_clientW = (rc.right - rc.left);
g_clientH = (rc.bottom - rc.top);

g_font = CreateFontA(g_style.fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, 
                     ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                     DEFAULT_QUALITY, FF_DONTCARE, "Consolas");
g_oldFont = SelectObject(hdc, g_font);
g_fontBase = glGenLists(256);
wglUseFontBitmaps(hdc, 0, 256, g_fontBase);

g_lastFrameTime = std::chrono::high_resolution_clock::now();

g_statsPanel = CreatePanel("Stats", 10, 10, 300, 250);
SetPanelCallback(g_statsPanel, Internal::RenderStatsWindow);

g_logPanel = CreatePanel("Log", 10, 270, 600, 300);
SetPanelCallback(g_logPanel, Internal::RenderLogWindow);
SetPanelVisible(g_logPanel, false);

g_profilerPanel = CreatePanel("Profiler", 320, 10, 450, 350);
SetPanelCallback(g_profilerPanel, Internal::RenderProfilerWindow);
SetPanelVisible(g_profilerPanel, false);
```

}

void Shutdown() {
HDC hdc = (HDC)Titan::Platform::Window::GetNativeHandle();
if (g_oldFont) SelectObject(hdc, g_oldFont);
if (g_font) DeleteObject(g_font);
if (g_fontBase) glDeleteLists(g_fontBase, 256);

```
for (auto* panel : g_panels) {
    delete panel;
}
g_panels.clear();

g_oldFont = nullptr;
g_font = nullptr;
g_fontBase = 0;
```

}

void Begin() {
if (!g_visible) return;

```
auto now = std::chrono::high_resolution_clock::now();
float delta = std::chrono::duration<float>(now - g_lastFrameTime).count();
g_lastFrameTime = now;
g_frameTime = delta * 1000.0f;
g_fps = 1.0f / delta;

g_fpsHistory.push_back(g_fps);
if (g_fpsHistory.size() > MAX_METRIC_HISTORY) {
    g_fpsHistory.erase(g_fpsHistory.begin());
}

g_frameTimeHistory.push_back(g_frameTime);
if (g_frameTimeHistory.size() > MAX_METRIC_HISTORY) {
    g_frameTimeHistory.erase(g_frameTimeHistory.begin());
}

HWND hwnd = (HWND)Titan::Platform::Window::GetNativeWindowHandle();
RECT rc;
if (hwnd) GetClientRect(hwnd, &rc);
g_clientW = (rc.right - rc.left);
g_clientH = (rc.bottom - rc.top);

glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadIdentity();
glOrtho(0, g_clientW, g_clientH, 0, -1, 1);
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();

glDisable(GL_DEPTH_TEST);
glDisable(GL_LIGHTING);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

int mx = Titan::Input::Manager::GetMouseX();
int my = Titan::Input::Manager::GetMouseY();
bool lmbDown = Titan::Input::Manager::IsKeyDown(VK_LBUTTON);
bool lmbPressed = Titan::Input::Manager::IsKeyPressed(VK_LBUTTON);

if (lmbPressed && !g_draggedPanel) {
    for (auto it = g_panels.rbegin(); it != g_panels.rend(); ++it) {
        Panel* p = *it;
        if (!p->visible || !p->draggable) continue;
        
        int titleHeight = g_style.fontSize + g_style.padding * 2;
        if (mx >= p->x && mx <= p->x + p->width && 
            my >= p->y && my <= p->y + titleHeight) {
            g_draggedPanel = p;
            g_dragOffsetX = mx - p->x;
            g_dragOffsetY = my - p->y;
            
            g_panels.erase(std::next(it).base());
            g_panels.push_back(p);
            break;
        }
    }
}

if (g_draggedPanel) {
    if (lmbDown) {
        g_draggedPanel->x = mx - g_dragOffsetX;
        g_draggedPanel->y = my - g_dragOffsetY;
    } else {
        g_draggedPanel = nullptr;
    }
}
```

}

void End() {
if (!g_visible) return;

```
for (Panel* panel : g_panels) {
    if (!panel->visible) continue;
    
    int titleHeight = g_style.fontSize + g_style.padding * 2;
    int contentY = panel->y + titleHeight;
    int contentHeight = panel->height - titleHeight;
    
    if (panel->collapsed) contentHeight = 0;
    
    Rect(panel->x, panel->y, panel->width, titleHeight,
         g_style.panelColor.r * 1.2f, g_style.panelColor.g * 1.2f, 
         g_style.panelColor.b * 1.2f, g_style.panelColor.a);
    
    if (!panel->collapsed) {
        Rect(panel->x, contentY, panel->width, contentHeight,
             g_style.panelColor.r, g_style.panelColor.g,
             g_style.panelColor.b, g_style.panelColor.a);
    }
    
    RectOutline(panel->x, panel->y, panel->width, 
               titleHeight + contentHeight,
               g_style.borderColor.r, g_style.borderColor.g,
               g_style.borderColor.b, g_style.borderColor.a);
    
    Text(panel->x + g_style.padding, 
         panel->y + g_style.padding + g_style.fontSize,
         panel->title.c_str());
    
    if (panel->collapsible) {
        int btnX = panel->x + panel->width - 20;
        int btnY = panel->y + 5;
        Text(btnX, btnY + g_style.fontSize, panel->collapsed ? "+" : "-");
        
        int mx = Titan::Input::Manager::GetMouseX();
        int my = Titan::Input::Manager::GetMouseY();
        if (mx >= btnX && mx <= btnX + 15 && my >= btnY && my <= btnY + 15) {
            if (Titan::Input::Manager::IsKeyPressed(VK_LBUTTON)) {
                panel->collapsed = !panel->collapsed;
            }
        }
    }
    
    if (!panel->collapsed && panel->renderCallback) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(panel->x, g_clientH - (contentY + contentHeight), 
                 panel->width, contentHeight);
        
        panel->renderCallback();
        
        glDisable(GL_SCISSOR_TEST);
    }
}

glEnable(GL_DEPTH_TEST);
glMatrixMode(GL_MODELVIEW);
glPopMatrix();
glMatrixMode(GL_PROJECTION);
glPopMatrix();
```

}

void Text(int x, int y, const char* text) {
if (!text) return;
glColor4f(g_style.textColor.r, g_style.textColor.g,
g_style.textColor.b, g_style.textColor.a);
glRasterPos2i(x, y);
glListBase(g_fontBase);
glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void TextColored(int x, int y, float r, float g, float b, float a, const char* text) {
if (!text) return;
glColor4f(r, g, b, a);
glRasterPos2i(x, y);
glListBase(g_fontBase);
glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void Rect(int x, int y, int w, int h, float r, float g, float b, float a) {
glColor4f(r, g, b, a);
glBegin(GL_QUADS);
glVertex2i(x, y);
glVertex2i(x + w, y);
glVertex2i(x + w, y + h);
glVertex2i(x, y + h);
glEnd();
}

void RectOutline(int x, int y, int w, int h, float r, float g, float b, float a) {
glColor4f(r, g, b, a);
glBegin(GL_LINE_LOOP);
glVertex2i(x, y);
glVertex2i(x + w, y);
glVertex2i(x + w, y + h);
glVertex2i(x, y + h);
glEnd();
}

void Line(int x1, int y1, int x2, int y2, float r, float g, float b, float a) {
glColor4f(r, g, b, a);
glBegin(GL_LINES);
glVertex2i(x1, y1);
glVertex2i(x2, y2);
glEnd();
}

bool Button(int x, int y, int w, int h, const char* text) {
int mx = Titan::Input::Manager::GetMouseX();
int my = Titan::Input::Manager::GetMouseY();
bool inside = (mx >= x && mx <= x + w && my >= y && my <= y + h);
bool clicked = false;

```
Style::Color color = g_style.buttonColor;
if (inside) {
    if (Titan::Input::Manager::IsKeyDown(VK_LBUTTON)) {
        color = g_style.buttonActiveColor;
    } else {
        color = g_style.buttonHoverColor;
    }
    
    if (Titan::Input::Manager::IsKeyPressed(VK_LBUTTON)) {
        clicked = true;
    }
}

Rect(x, y, w, h, color.r, color.g, color.b, color.a);
RectOutline(x, y, w, h, 
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

int textW = GetTextWidth(text);
Text(x + (w - textW) / 2, y + h / 2 + g_style.fontSize / 3, text);

return clicked;
```

}

bool Checkbox(int x, int y, const char* label, bool* value) {
int size = g_style.fontSize;
int mx = Titan::Input::Manager::GetMouseX();
int my = Titan::Input::Manager::GetMouseY();
bool inside = (mx >= x && mx <= x + size && my >= y && my <= y + size);

```
if (inside && Titan::Input::Manager::IsKeyPressed(VK_LBUTTON)) {
    *value = !*value;
}

Style::Color color = inside ? g_style.buttonHoverColor : g_style.buttonColor;
Rect(x, y, size, size, color.r, color.g, color.b, color.a);
RectOutline(x, y, size, size,
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

if (*value) {
    Line(x + 3, y + size/2, x + size/2, y + size - 3, 1.0f, 1.0f, 1.0f, 1.0f);
    Line(x + size/2, y + size - 3, x + size - 3, y + 3, 1.0f, 1.0f, 1.0f, 1.0f);
}

Text(x + size + g_style.padding, y + size - 2, label);

return *value;
```

}

bool Slider(int x, int y, int w, const char* label, float* value, float min, float max) {
int h = g_style.fontSize + 4;
int labelW = GetTextWidth(label);
int sliderX = x + labelW + g_style.padding;
int sliderW = w - labelW - g_style.padding;

```
Text(x, y + h - 2, label);

Rect(sliderX, y, sliderW, h, 
     g_style.bgColor.r, g_style.bgColor.g, 
     g_style.bgColor.b, g_style.bgColor.a);
RectOutline(sliderX, y, sliderW, h,
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

float t = (*value - min) / (max - min);
t = std::max(0.0f, std::min(1.0f, t));
int handleX = sliderX + (int)(t * sliderW);

Rect(handleX - 3, y - 2, 6, h + 4,
     g_style.buttonHoverColor.r, g_style.buttonHoverColor.g,
     g_style.buttonHoverColor.b, g_style.buttonHoverColor.a);

int mx = Titan::Input::Manager::GetMouseX();
int my = Titan::Input::Manager::GetMouseY();
if (Titan::Input::Manager::IsKeyDown(VK_LBUTTON)) {
    if (mx >= sliderX && mx <= sliderX + sliderW && 
        my >= y - 5 && my <= y + h + 5) {
        t = (float)(mx - sliderX) / sliderW;
        t = std::max(0.0f, std::min(1.0f, t));
        *value = min + t * (max - min);
    }
}

return false;
```

}

void Graph(int x, int y, int w, int h, const float* values, int count, float min, float max) {
if (count < 2) return;

```
Rect(x, y, w, h, 0.05f, 0.05f, 0.05f, 0.8f);
RectOutline(x, y, w, h, 
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

float range = max - min;
if (range < 0.001f) range = 1.0f;

glColor4f(0.3f, 0.8f, 0.3f, 1.0f);
glBegin(GL_LINE_STRIP);
for (int i = 0; i < count; ++i) {
    float t = (float)i / (count - 1);
    float normalized = (values[i] - min) / range;
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    int px = x + (int)(t * w);
    int py = y + h - (int)(normalized * h);
    glVertex2i(px, py);
}
glEnd();
```

}

void Log(LogLevel level, const char* format, …) {
char buffer[1024];
va_list args;
va_start(args, format);
vsnprintf(buffer, sizeof(buffer), format, args);
va_end(args);

```
LogEntry entry;
entry.level = level;
entry.message = buffer;
entry.timestamp = GetTimestamp();

g_logs.push_back(entry);
if (g_logs.size() > MAX_LOGS) {
    g_logs.erase(g_logs.begin());
}
```

}

void LogInfo(const char* format, …) {
char buffer[1024];
va_list args;
va_start(args, format);
vsnprintf(buffer, sizeof(buffer), format, args);
va_end(args);
Log(LogLevel::Info, “%s”, buffer);
}

void LogWarning(const char* format, …) {
char buffer[1024];
va_list args;
va_start(args, format);
vsnprintf(buffer, sizeof(buffer), format, args);
va_end(args);
Log(LogLevel::Warning, “%s”, buffer);
}

void LogError(const char* format, …) {
char buffer[1024];
va_list args;
va_start(args, format);
vsnprintf(buffer, sizeof(buffer), format, args);
va_end(args);
Log(LogLevel::Error, “%s”, buffer);
}

const std::vector<LogEntry>& GetLogs() {
return g_logs;
}

void ClearLogs() {
g_logs.clear();
}

ScopedTimer::ScopedTimer(const char* name)
: m_name(name) {
m_start = std::chrono::high_resolution_clock::now();
}

ScopedTimer::~ScopedTimer() {
auto end = std::chrono::high_resolution_clock::now();
float ms = std::chrono::duration<float, std::milli>(end - m_start).count();

```
auto& data = g_profileData[m_name];
if (data.callCount == 0) {
    data.minTime = ms;
    data.maxTime = ms;
    data.avgTime = ms;
} else {
    data.minTime = std::min(data.minTime, ms);
    data.maxTime = std::max(data.maxTime, ms);
    data.avgTime = (data.avgTime * data.callCount + ms) / (data.callCount + 1);
}
data.callCount++;

g_timerHistory[m_name].push_back(ms);
if (g_timerHistory[m_name].size() > MAX_METRIC_HISTORY) {
    g_timerHistory[m_name].erase(g_timerHistory[m_name].begin());
}
```

}

const std::unordered_map<std::string, ProfileData>& GetProfileData() {
return g_profileData;
}

void ResetProfileData() {
g_profileData.clear();
g_timerHistory.clear();
}

void TrackMetric(const char* name, float value) {
g_metrics[name].push_back(value);
if (g_metrics[name].size() > MAX_METRIC_HISTORY) {
g_metrics[name].erase(g_metrics[name].begin());
}
}

float GetMetric(const char* name) {
auto it = g_metrics.find(name);
if (it != g_metrics.end() && !it->second.empty()) {
return it->second.back();
}
return 0.0f;
}

const std::unordered_map<std::string, std::vector<float>>& GetMetricHistory() {
return g_metrics;
}

Panel* CreatePanel(const char* title, int x, int y, int w, int h) {
Panel* panel = new Panel();
panel->title = title;
panel->x = x;
panel->y = y;
panel->width = w;
panel->height = h;
panel->visible = true;
panel->draggable = true;
panel->collapsible = true;
panel->collapsed = false;
g_panels.push_back(panel);
return panel;
}

void DestroyPanel(Panel* panel) {
auto it = std::find(g_panels.begin(), g_panels.end(), panel);
if (it != g_panels.end()) {
g_panels.erase(it);
delete panel;
}
}

void SetPanelVisible(Panel* panel, bool visible) {
if (panel) panel->visible = visible;
}

void SetPanelCallback(Panel* panel, std::function<void()> callback) {
if (panel) panel->renderCallback = callback;
}

void SetStyle(const Style& style) {
g_style = style;
}

const Style& GetStyle() {
return g_style;
}

void ShowStatsWindow(bool show) {
g_showStats = show;
if (g_statsPanel) SetPanelVisible(g_statsPanel, show);
}

void ShowLogWindow(bool show) {
g_showLog = show;
if (g_logPanel) SetPanelVisible(g_logPanel, show);
}

void ShowProfilerWindow(bool show) {
g_showProfiler = show;
if (g_profilerPanel) SetPanelVisible(g_profilerPanel, show);
}

bool IsVisible() {
return g_visible;
}

void SetVisible(bool visible) {
g_visible = visible;
}

namespace Internal {
void RenderStatsWindow() {
if (!g_statsPanel) return;

```
    int x = g_statsPanel->x + g_style.padding;
    int y = g_statsPanel->y + g_style.fontSize + g_style.padding * 3;
    int lineH = g_style.fontSize + g_style.margin;
    
    char buf[256];
    
    sprintf(buf, "FPS: %.1f", g_fps);
    Text(x, y, buf);
    y += lineH;
    
    sprintf(buf, "Frame Time: %.2f ms", g_frameTime);
    Text(x, y, buf);
    y += lineH;
    
    sprintf(buf, "Resolution: %dx%d", g_clientW, g_clientH);
    Text(x, y, buf);
    y += lineH;
    
    sprintf(buf, "Logs: %zu", g_logs.size());
    Text(x, y, buf);
    y += lineH;
    
    sprintf(buf, "Profile Entries: %zu", g_profileData.size());
    Text(x, y, buf);
    y += lineH * 2;
    
    if (!g_fpsHistory.empty()) {
        Text(x, y - lineH, "FPS Graph:");
        Graph(x, y, 280, 60, g_fpsHistory.data(), (int)g_fpsHistory.size(), 0.0f, 120.0f);
        y += 65;
    }
    
    if (!g_frameTimeHistory.empty()) {
        Text(x, y, "Frame Time (ms):");
        y += lineH;
        Graph(x, y, 280, 60, g_frameTimeHistory.data(), (int)g_frameTimeHistory.size(), 0.0f, 33.0f);
    }
}

void RenderLogWindow() {
    if (!g_logPanel) return;
    
    int x = g_logPanel->x + g_style.padding;
    int y = g_logPanel->y + g_style.fontSize + g_style.padding * 3;
    int lineH = g_style.fontSize + g_style.margin;
    
    int visibleLines = (g_logPanel->height - (g_style.fontSize + g_style.padding * 3)) / lineH;
    int startIdx = std::max(0, (int)g_logs.size() - visibleLines);
    
    for (size_t i = startIdx; i < g_logs.size(); ++i) {
        const auto& log = g_logs[i];
        
        Style::Color color;
        switch (log.level) {
            case LogLevel::Info: color = g_style.logInfo; break;
            case LogLevel::Warning: color = g_style.logWarning; break;
            case LogLevel::Error: color = g_style.logError; break;
        }
        
        char buf[512];
        sprintf(buf, "[%s] %s", log.timestamp.c_str(), log.message.c_str());
        TextColored(x, y, color.r, color.g, color.b, color.a, buf);
        y += lineH;
    }
}

void RenderProfilerWindow() {
    if (!g_profilerPanel) return;
    
    int x = g_profilerPanel->x + g_style.padding;
    int y = g_profilerPanel->y + g_style.fontSize + g_style.padding * 3;
    int lineH = g_style.fontSize + g_style.margin;
    
    Text(x, y, "Function");
    Text(x + 200, y, "Avg");
    Text(x + 260, y, "Min");
    Text(x + 320, y, "Max");
    Text(x + 380, y, "Calls");
    y += lineH * 2;
    
    for (const auto& entry : g_profileData) {
        char buf[256];
        sprintf(buf, "%-24s", entry.first.c_str());
        Text(x, y, buf);
        
        sprintf(buf, "%.2f", entry.second.avgTime);
        Text(x + 200, y, buf);
        
        sprintf(buf, "%.2f", entry.second.minTime);
        Text(x + 260, y, buf);
        
        sprintf(buf, "%.2f", entry.second.maxTime);
        Text(x + 320, y, buf);
        
        sprintf(buf, "%u", entry.second.callCount);
        Text(x + 380, y, buf);
        
        y += lineH;
    }
}
```

}

}
