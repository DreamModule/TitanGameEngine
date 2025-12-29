#include “DebugSystem.hpp”
#include “../Platform/Window.hpp”
#include “../Input/InputManager.hpp”
#include “../Input/Joystick.hpp”
#include “../Input/InputBuffer.hpp”
#include <windows.h>
#include <gl/GL.h>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <algorithm>
#include <fstream>

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
bool g_showInput = false;
bool g_showConsole = false;
bool g_showPerformanceOverlay = false;

Panel* g_statsPanel = nullptr;
Panel* g_logPanel = nullptr;
Panel* g_profilerPanel = nullptr;
Panel* g_inputPanel = nullptr;
Panel* g_consolePanel = nullptr;

float g_fps = 0.0f;
float g_frameTime = 0.0f;
std::chrono::high_resolution_clock::time_point g_lastFrameTime;
std::vector<float> g_fpsHistory;
std::vector<float> g_frameTimeHistory;

int g_logScrollOffset = 0;
int g_profilerScrollOffset = 0;
int g_consoleScrollOffset = 0;
char g_consoleInputBuffer[256] = {0};
std::vector<std::string> g_consoleHistory;

bool g_logFilterInfo = true;
bool g_logFilterWarning = true;
bool g_logFilterError = true;
char g_logSearchBuffer[128] = {0};

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

g_statsPanel = CreatePanel("Stats", 10, 10, 300, 280);
SetPanelCallback(g_statsPanel, Internal::RenderStatsWindow);

g_logPanel = CreatePanel("Log", 10, 300, 600, 400);
SetPanelCallback(g_logPanel, Internal::RenderLogWindow);
SetPanelVisible(g_logPanel, false);

g_profilerPanel = CreatePanel("Profiler", 320, 10, 450, 380);
SetPanelCallback(g_profilerPanel, Internal::RenderProfilerWindow);
SetPanelVisible(g_profilerPanel, false);

g_inputPanel = CreatePanel("Input Monitor", 780, 10, 480, 450);
SetPanelCallback(g_inputPanel, Internal::RenderInputWindow);
SetPanelVisible(g_inputPanel, false);

g_consolePanel = CreatePanel("Console", 320, 300, 600, 300);
SetPanelCallback(g_consolePanel, Internal::RenderConsoleWindow);
SetPanelVisible(g_consolePanel, false);
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
if (!g_visible && !g_showPerformanceOverlay) return;

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

if (!g_visible) return;

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
if (g_showPerformanceOverlay) {
Internal::RenderPerformanceOverlay();
}

```
if (!g_visible) {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    return;
}

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

void Graph(int x, int y, int w, int h, const float* values, int count, float min, float max, const char* label) {
if (count < 2) return;

```
Rect(x, y, w, h, 0.05f, 0.05f, 0.05f, 0.8f);
RectOutline(x, y, w, h, 
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

if (label) {
    Text(x + 4, y + 12, label);
}

glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
for (int i = 1; i < 5; ++i) {
    int ly = y + (h * i) / 5;
    Line(x, ly, x + w, ly, 0.2f, 0.2f, 0.2f, 0.5f);
}

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

char minLabel[32], maxLabel[32];
sprintf(minLabel, "%.1f", min);
sprintf(maxLabel, "%.1f", max);
TextColored(x + 2, y + h - 2, 0.6f, 0.6f, 0.6f, 1.0f, minLabel);
TextColored(x + 2, y + 12, 0.6f, 0.6f, 0.6f, 1.0f, maxLabel);
```

}

bool TextInput(int x, int y, int w, const char* label, char* buffer, size_t bufferSize) {
int h = g_style.fontSize + 4;
int labelW = GetTextWidth(label);
int inputX = x + labelW + g_style.padding;
int inputW = w - labelW - g_style.padding;

```
Text(x, y + h - 2, label);

Rect(inputX, y, inputW, h, 
     g_style.bgColor.r * 0.5f, g_style.bgColor.g * 0.5f, 
     g_style.bgColor.b * 0.5f, g_style.bgColor.a);
RectOutline(inputX, y, inputW, h,
            g_style.borderColor.r, g_style.borderColor.g,
            g_style.borderColor.b, g_style.borderColor.a);

Text(inputX + 4, y + h - 2, buffer);

return false;
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

void ExportLogsToFile(const char* filename) {
std::ofstream file(filename);
if (!file.is_open()) {
LogError(“Failed to export logs to %s”, filename);
return;
}

```
for (const auto& log : g_logs) {
    const char* levelStr = "INFO";
    if (log.level == LogLevel::Warning) levelStr = "WARN";
    else if (log.level == LogLevel::Error) levelStr = "ERROR";
    
    file << "[" << log.timestamp << "] [" << levelStr << "] " << log.message << "\n";
}

file.close();
LogInfo("Logs exported to %s", filename);
```

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
panel->scrollOffset = 0;
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

void ShowInputWindow(bool show) {
g_showInput = show;
if (g_inputPanel) SetPanelVisible(g_inputPanel, show);
}

void ShowConsoleWindow(bool show) {
g_showConsole = show;
if (g_consolePanel) SetPanelVisible(g_consolePanel, show);
}

void ShowPerformanceOverlay(bool show) {
g_showPerformanceOverlay = show;
}

bool IsVisible() {
return g_visible;
}

void SetVisible(bool visible) {
g_visible = visible;
}

void TakeScreenshot(const char* filename) {
int width = g_clientW;
int height = g_clientH;

```
unsigned char* pixels = new unsigned char[width * height * 3];
glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

FILE* file = fopen(filename, "wb");
if (file) {
    fprintf(file, "P6\n%d %d\n255\n", width, height);
    for (int y = height - 1; y >= 0; --y) {
        fwrite(&pixels[y * width * 3], 1, width * 3, file);
    }
    fclose(file);
    LogInfo("Screenshot saved: %s", filename);
} else {
    LogError("Failed to save screenshot: %s", filename);
}

delete[] pixels;
```

}

void SaveSettings(const char* filename) {
std::ofstream file(filename);
if (!file.is_open()) {
LogError(“Failed to save settings to %s”, filename);
return;
}

```
file << "# Debug System Settings\n";
for (const auto* panel : g_panels) {
    file << "panel," << panel->title << "," << panel->x << "," << panel->y 
         << "," << panel->width << "," << panel->height << "," 
         << panel->visible << "," << panel->collapsed << "\n";
}

file.close();
LogInfo("Settings saved to %s", filename);
```

}

void LoadSettings(const char* filename) {
std::ifstream file(filename);
if (!file.is_open()) {
LogWarning(“Failed to load settings from %s”, filename);
return;
}

```
std::string line;
while (std::getline(file, line)) {
    if (line.empty() ||line[0] == '#') continue;
        
        size_t pos = 0;
        std::string token;
        std::vector<std::string> tokens;
        
        while ((pos = line.find(',')) != std::string::npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos + 1);
        }
        tokens.push_back(line);
        
        if (tokens.size() >= 8 && tokens[0] == "panel") {
            for (auto* panel : g_panels) {
                if (panel->title == tokens[1]) {
                    panel->x = std::stoi(tokens[2]);
                    panel->y = std::stoi(tokens[3]);
                    panel->width = std::stoi(tokens[4]);
                    panel->height = std::stoi(tokens[5]);
                    panel->visible = (tokens[6] == "1");
                    panel->collapsed = (tokens[7] == "1");
                    break;
                }
            }
        }
    }

    file.close();
    LogInfo("Settings loaded from %s", filename);
}

namespace Internal {
    void RenderStatsWindow() {
        if (!g_statsPanel) return;

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
            Graph(x, y, 280, 60, g_fpsHistory.data(), (int)g_fpsHistory.size(), 0.0f, 120.0f, "FPS");
            y += 70;
        }
        
        if (!g_frameTimeHistory.empty()) {
            Graph(x, y, 280, 60, g_frameTimeHistory.data(), (int)g_frameTimeHistory.size(), 0.0f, 33.0f, "Frame Time (ms)");
        }
    }

    void RenderLogWindow() {
        if (!g_logPanel) return;
        
        int x = g_logPanel->x + g_style.padding;
        int y = g_logPanel->y + g_style.fontSize + g_style.padding * 3;
        int lineH = g_style.fontSize + g_style.margin;
        
        int filterY = y;
        Checkbox(x, filterY, "Info", &g_logFilterInfo);
        Checkbox(x + 80, filterY, "Warning", &g_logFilterWarning);
        Checkbox(x + 180, filterY, "Error", &g_logFilterError);
        
        if (Button(x + 280, filterY - 2, 80, 20, "Clear")) {
            ClearLogs();
        }
        
        if (Button(x + 370, filterY - 2, 80, 20, "Export")) {
            ExportLogsToFile("debug_logs.txt");
        }
        
        y += lineH * 2;
        
        int visibleLines = (g_logPanel->height - (y - g_logPanel->y)) / lineH;
        
        std::vector<const LogEntry*> filteredLogs;
        for (const auto& log : g_logs) {
            bool pass = false;
            if (log.level == LogLevel::Info && g_logFilterInfo) pass = true;
            if (log.level == LogLevel::Warning && g_logFilterWarning) pass = true;
            if (log.level == LogLevel::Error && g_logFilterError) pass = true;
            
            if (pass) {
                if (strlen(g_logSearchBuffer) > 0) {
                    if (log.message.find(g_logSearchBuffer) != std::string::npos) {
                        filteredLogs.push_back(&log);
                    }
                } else {
                    filteredLogs.push_back(&log);
                }
            }
        }
        
        int startIdx = std::max(0, (int)filteredLogs.size() - visibleLines);
        
        for (size_t i = startIdx; i < filteredLogs.size(); ++i) {
            const auto* log = filteredLogs[i];
            
            Style::Color color;
            switch (log->level) {
                case LogLevel::Info: color = g_style.logInfo; break;
                case LogLevel::Warning: color = g_style.logWarning; break;
                case LogLevel::Error: color = g_style.logError; break;
            }
            
            char buf[512];
            sprintf(buf, "[%s] %s", log->timestamp.c_str(), log->message.c_str());
            TextColored(x, y, color.r, color.g, color.b, color.a, buf);
            y += lineH;
        }
    }

    void RenderProfilerWindow() {
        if (!g_profilerPanel) return;
        
        int x = g_profilerPanel->x + g_style.padding;
        int y = g_profilerPanel->y + g_style.fontSize + g_style.padding * 3;
        int lineH = g_style.fontSize + g_style.margin;
        
        if (Button(x, y - 2, 100, 20, "Reset Stats")) {
            ResetProfileData();
        }
        
        y += lineH * 2;
        
        Text(x, y, "Function");
        Text(x + 200, y, "Avg");
        Text(x + 260, y, "Min");
        Text(x + 320, y, "Max");
        Text(x + 380, y, "Calls");
        y += lineH * 2;
        
        int maxLines = (g_profilerPanel->height - (y - g_profilerPanel->y)) / lineH;
        int count = 0;
        
        for (const auto& entry : g_profileData) {
            if (count >= maxLines) break;
            
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
            count++;
        }
    }

    void RenderInputWindow() {
        if (!g_inputPanel) return;
        
        int x = g_inputPanel->x + g_style.padding;
        int y = g_inputPanel->y + g_style.fontSize + g_style.padding * 3;
        int lineH = g_style.fontSize + g_style.margin;
        
        int mx = Titan::Input::Manager::GetMouseX();
        int my = Titan::Input::Manager::GetMouseY();
        
        Text(x, y, "=== Mouse ===");
        y += lineH;
        
        char buf[256];
        sprintf(buf, "Position: %d, %d", mx, my);
        Text(x, y, buf);
        y += lineH;
        
        bool lmb = Titan::Input::Manager::IsKeyDown(VK_LBUTTON);
        bool rmb = Titan::Input::Manager::IsKeyDown(VK_RBUTTON);
        bool mmb = Titan::Input::Manager::IsKeyDown(VK_MBUTTON);
        
        sprintf(buf, "LMB: %s  RMB: %s  MMB: %s", 
                lmb ? "DOWN" : "UP", 
                rmb ? "DOWN" : "UP", 
                mmb ? "DOWN" : "UP");
        Text(x, y, buf);
        y += lineH * 2;
        
        Text(x, y, "=== Keyboard ===");
        y += lineH;
        
        const char* keys[] = {"W", "A", "S", "D", "Space", "Shift", "Ctrl", "Tab", "Esc"};
        uint32_t vks[] = {'W', 'A', 'S', 'D', VK_SPACE, VK_SHIFT, VK_CONTROL, VK_TAB, VK_ESCAPE};
        
        for (int i = 0; i < 9; ++i) {
            bool down = Titan::Input::Manager::IsKeyDown(vks[i]);
            Style::Color color = down ? g_style.inputActive : g_style.inputInactive;
            
            int bx = x + (i % 3) * 60;
            int by = y + (i / 3) * 25;
            
            Rect(bx, by, 50, 20, color.r, color.g, color.b, color.a);
            RectOutline(bx, by, 50, 20, 
                       g_style.borderColor.r, g_style.borderColor.g,
                       g_style.borderColor.b, g_style.borderColor.a);
            Text(bx + 15, by + 15, keys[i]);
        }
        y += 80;
        
        Text(x, y, "=== Gamepad ===");
        y += lineH;
        
        bool connected = Titan::Input::Joystick::IsControllerConnected(0);
        sprintf(buf, "Controller 0: %s", connected ? "CONNECTED" : "DISCONNECTED");
        Text(x, y, buf);
        y += lineH;
        
        if (connected) {
            float lx = Titan::Input::Joystick::GetLeftAxisX(0);
            float ly = Titan::Input::Joystick::GetLeftAxisY(0);
            
            sprintf(buf, "Left Stick: %.2f, %.2f", lx, ly);
            Text(x, y, buf);
            y += lineH;
            
            int centerX = x + 100;
            int centerY = y + 40;
            int radius = 30;
            
            Rect(centerX - radius, centerY - radius, radius * 2, radius * 2, 
                 0.2f, 0.2f, 0.2f, 0.8f);
            RectOutline(centerX - radius, centerY - radius, radius * 2, radius * 2,
                       g_style.borderColor.r, g_style.borderColor.g,
                       g_style.borderColor.b, g_style.borderColor.a);
            
            int stickX = centerX + (int)(lx * radius);
            int stickY = centerY - (int)(ly * radius);
            
            Rect(stickX - 5, stickY - 5, 10, 10, 
                 g_style.inputActive.r, g_style.inputActive.g,
                 g_style.inputActive.b, g_style.inputActive.a);
            
            y += 85;
        }
        
        Text(x, y, "=== Virtual Joystick ===");
        y += lineH;
        
        auto& vj = Titan::Input::Joystick::GetVirtual();
        sprintf(buf, "Enabled: %s", vj.enabled ? "YES" : "NO");
        Text(x, y, buf);
        y += lineH;
        
        if (vj.enabled) {
            sprintf(buf, "Stick: %.2f, %.2f", vj.stickX, vj.stickY);
            Text(x, y, buf);
        }
    }

    void RenderConsoleWindow() {
        if (!g_consolePanel) return;
        
        int x = g_consolePanel->x + g_style.padding;
        int y = g_consolePanel->y + g_style.fontSize + g_style.padding * 3;
        int lineH = g_style.fontSize + g_style.margin;
        
        Text(x, y, "Console commands:");
        y += lineH;
        
        TextColored(x, y, 0.7f, 0.7f, 0.7f, 1.0f, "- clear: Clear console history");
        y += lineH;
        TextColored(x, y, 0.7f, 0.7f, 0.7f, 1.0f, "- fps: Toggle FPS display");
        y += lineH;
        TextColored(x, y, 0.7f, 0.7f, 0.7f, 1.0f, "- screenshot: Take screenshot");
        y += lineH * 2;
        
        int visibleLines = (g_consolePanel->height - (y - g_consolePanel->y) - 30) / lineH;
        int startIdx = std::max(0, (int)g_consoleHistory.size() - visibleLines);
        
        for (size_t i = startIdx; i < g_consoleHistory.size(); ++i) {
            Text(x, y, g_consoleHistory[i].c_str());
            y += lineH;
        }
        
        int inputY = g_consolePanel->y + g_consolePanel->height - 30;
        Text(x, inputY + 15, "> ");
        TextInput(x + 15, inputY, g_consolePanel->width - 30, "", g_consoleInputBuffer, sizeof(g_consoleInputBuffer));
    }

    void RenderPerformanceOverlay() {
        int x = g_clientW - 150;
        int y = 10;
        int w = 140;
        int h = 80;
        
        Rect(x, y, w, h, 
             g_style.panelColor.r, g_style.panelColor.g,
             g_style.panelColor.b, g_style.panelColor.a * 0.7f);
        RectOutline(x, y, w, h,
                   g_style.borderColor.r, g_style.borderColor.g,
                   g_style.borderColor.b, g_style.borderColor.a);
        
        int lineH = g_style.fontSize + g_style.margin;
        x += g_style.padding;
        y += g_style.padding + g_style.fontSize;
        
        char buf[64];
        sprintf(buf, "FPS: %.1f", g_fps);
        Text(x, y, buf);
        y += lineH;
        
        sprintf(buf, "Frame: %.2f ms", g_frameTime);
        Text(x, y, buf);
        y += lineH;
        
        Style::Color color = g_fps > 60 ? g_style.inputActive : 
                            g_fps > 30 ? g_style.logWarning : g_style.logError;
        
        int barX = x;
        int barY = y;
        int barW = 130;
        int barH = 10;
        
        Rect(barX, barY, barW, barH, 0.2f, 0.2f, 0.2f, 0.8f);
        
        float fpsNorm = std::min(g_fps / 120.0f, 1.0f);
        int fillW = (int)(barW * fpsNorm);
        Rect(barX, barY, fillW, barH, color.r, color.g, color.b, color.a);
    }
}

}

```
