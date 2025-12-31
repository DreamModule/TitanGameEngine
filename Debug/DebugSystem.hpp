#pragma once
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <unordered_map>

namespace Titan::Debug {

struct Style {
struct Color {
float r, g, b, a;
Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
: r(r), g(g), b(b), a(a) {}
};

```
Color textColor{1.0f, 1.0f, 1.0f, 1.0f};
Color bgColor{0.1f, 0.1f, 0.1f, 0.9f};
Color buttonColor{0.2f, 0.2f, 0.2f, 1.0f};
Color buttonHoverColor{0.3f, 0.5f, 0.9f, 1.0f};
Color buttonActiveColor{0.4f, 0.6f, 1.0f, 1.0f};
Color panelColor{0.15f, 0.15f, 0.15f, 0.95f};
Color borderColor{0.4f, 0.4f, 0.4f, 1.0f};
Color logInfo{0.7f, 0.7f, 0.7f, 1.0f};
Color logWarning{1.0f, 0.8f, 0.2f, 1.0f};
Color logError{1.0f, 0.3f, 0.3f, 1.0f};
Color inputActive{0.3f, 0.9f, 0.3f, 1.0f};
Color inputInactive{0.3f, 0.3f, 0.3f, 1.0f};

int fontSize = 16;
int padding = 6;
int margin = 4;
```

};

enum class LogLevel {
Info,
Warning,
Error
};

struct LogEntry {
LogLevel level;
std::string message;
std::string timestamp;
};

void Log(LogLevel level, const char* format, …);
void LogInfo(const char* format, …);
void LogWarning(const char* format, …);
void LogError(const char* format, …);
const std::vector<LogEntry>& GetLogs();
void ClearLogs();
void ExportLogsToFile(const char* filename);

class ScopedTimer {
public:
ScopedTimer(const char* name);
~ScopedTimer();
private:
const char* m_name;
std::chrono::high_resolution_clock::time_point m_start;
};

struct ProfileData {
float avgTime;
float minTime;
float maxTime;
uint32_t callCount;
};

const std::unordered_map<std::string, ProfileData>& GetProfileData();
void ResetProfileData();

#define PROFILE_SCOPE(name) Titan::Debug::ScopedTimer *timer*##**LINE**(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(**FUNCTION**)

void TrackMetric(const char* name, float value);
float GetMetric(const char* name);
const std::unordered_map<std::string, std::vector<float>>& GetMetricHistory();

struct Panel {
std::string title;
int x, y, width, height;
bool visible;
bool draggable;
bool collapsible;
bool collapsed;
int scrollOffset;
std::function<void()> renderCallback;
};

Panel* CreatePanel(const char* title, int x, int y, int w, int h);
void DestroyPanel(Panel* panel);
void SetPanelVisible(Panel* panel, bool visible);
void SetPanelCallback(Panel* panel, std::function<void()> callback);

void Text(int x, int y, const char* text);
void TextColored(int x, int y, float r, float g, float b, float a, const char* text);
void Rect(int x, int y, int w, int h, float r, float g, float b, float a);
void RectOutline(int x, int y, int w, int h, float r, float g, float b, float a);
void Line(int x1, int y1, int x2, int y2, float r, float g, float b, float a);

bool Button(int x, int y, int w, int h, const char* text);
bool Checkbox(int x, int y, const char* label, bool* value);
bool Slider(int x, int y, int w, const char* label, float* value, float min, float max);
void Graph(int x, int y, int w, int h, const float* values, int count, float min, float max, const char* label = nullptr);
bool TextInput(int x, int y, int w, const char* label, char* buffer, size_t bufferSize);

void Init();
void Shutdown();
void Begin();
void End();

void SetStyle(const Style& style);
const Style& GetStyle();

void ShowStatsWindow(bool show);
void ShowLogWindow(bool show);
void ShowProfilerWindow(bool show);
void ShowInputWindow(bool show);
void ShowConsoleWindow(bool show);
void ShowPerformanceOverlay(bool show);

bool IsVisible();
void SetVisible(bool visible);

void TakeScreenshot(const char* filename);

void SaveSettings(const char* filename);
void LoadSettings(const char* filename);

namespace Internal {
void RenderStatsWindow();
void RenderLogWindow();
void RenderProfilerWindow();
void RenderInputWindow();
void RenderConsoleWindow();
void RenderPerformanceOverlay();
}

}
