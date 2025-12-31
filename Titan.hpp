#ifndef TITAN_HPP
#define TITAN_HPP

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <gl/GL.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>
#include <chrono>
#include <random>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ws2_32.lib")

namespace Titan {

using EntityID = uint32_t;
using MeshHandle = uint32_t;
using MaterialHandle = uint32_t;
using TextureHandle = uint32_t;
using SoundHandle = uint32_t;
using ColliderID = uint32_t;
using ClientID = uint32_t;

constexpr EntityID INVALID_ENTITY = 0xFFFFFFFF;
constexpr MeshHandle INVALID_MESH = 0;
constexpr ClientID INVALID_CLIENT = 0xFFFFFFFF;

struct Vec2 {
    float x = 0, y = 0;
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    float Length() const { return sqrtf(x * x + y * y); }
    Vec2 Normalized() const { float l = Length(); return l > 0.0001f ? Vec2{x/l, y/l} : *this; }
};

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float LengthSq() const { return x * x + y * y + z * z; }
    Vec3 Normalized() const { float l = Length(); return l > 0.0001f ? Vec3{x/l, y/l, z/l} : *this; }
    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 Cross(const Vec3& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
};

struct Vec4 {
    float x = 0, y = 0, z = 0, w = 1;
    Vec4() = default;
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Mat4 {
    float m[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    
    static Mat4 Identity() { return Mat4(); }
    
    static Mat4 Perspective(float fov, float aspect, float nearZ, float farZ) {
        Mat4 r = {};
        float tanHalf = tanf(fov * 0.5f * 3.14159f / 180.0f);
        r.m[0] = 1.0f / (aspect * tanHalf);
        r.m[5] = 1.0f / tanHalf;
        r.m[10] = -(farZ + nearZ) / (farZ - nearZ);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
        r.m[15] = 0.0f;
        return r;
    }
    
    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).Normalized();
        Vec3 s = f.Cross(up).Normalized();
        Vec3 u = s.Cross(f);
        Mat4 r;
        r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
        r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
        r.m[12] = -s.Dot(eye);
        r.m[13] = -u.Dot(eye);
        r.m[14] = f.Dot(eye);
        return r;
    }
    
    static Mat4 Translate(const Vec3& v) {
        Mat4 r;
        r.m[12] = v.x; r.m[13] = v.y; r.m[14] = v.z;
        return r;
    }
    
    static Mat4 Scale(const Vec3& v) {
        Mat4 r;
        r.m[0] = v.x; r.m[5] = v.y; r.m[10] = v.z;
        return r;
    }
    
    static Mat4 RotateY(float rad) {
        Mat4 r;
        float c = cosf(rad), s = sinf(rad);
        r.m[0] = c; r.m[2] = s;
        r.m[8] = -s; r.m[10] = c;
        return r;
    }
};

struct RayHit {
    bool valid = false;
    float distance = 0;
    Vec3 point;
    Vec3 normal;
    EntityID entity = INVALID_ENTITY;
};

enum class KeyCode {
    None = 0,
    MouseLeft = 1, MouseRight = 2, MouseMiddle = 4,
    Backspace = 8, Tab = 9, Enter = 13, Shift = 16, Ctrl = 17, Alt = 18,
    Pause = 19, CapsLock = 20, Escape = 27, Space = 32,
    PageUp = 33, PageDown = 34, End = 35, Home = 36,
    Left = 37, Up = 38, Right = 39, Down = 40,
    Insert = 45, Delete = 46,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftShift = 160, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt,
    Tilde = 192
};

namespace Internal {
    inline HWND g_hwnd = nullptr;
    inline HDC g_hdc = nullptr;
    inline HGLRC g_hrc = nullptr;
    inline GLuint g_fontBase = 0;
    inline bool g_running = true;
    inline int g_width = 1280;
    inline int g_height = 720;
    inline bool g_keys[256] = {};
    inline bool g_keysDown[256] = {};
    inline bool g_keysUp[256] = {};
    inline Vec2 g_mousePos;
    inline Vec2 g_mouseDelta;
    inline bool g_mouseLocked = false;
    inline int g_mouseCenterX = 0;
    inline int g_mouseCenterY = 0;
    inline std::unordered_map<std::string, KeyCode> g_actions;
    inline float g_deltaTime = 0.016f;
    inline float g_totalTime = 0;
    inline int g_fps = 0;
    inline LARGE_INTEGER g_freq;
    inline LARGE_INTEGER g_lastTime;
    
    inline Vec3 g_camPos = {0, 2, 5};
    inline float g_camYaw = 0;
    inline float g_camPitch = 0;
    inline float g_camFOV = 70.0f;
    inline float g_camSens = 0.002f;
    inline float g_camSpeed = 5.0f;
    
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_CLOSE:
            case WM_DESTROY:
                g_running = false;
                PostQuitMessage(0);
                return 0;
            case WM_KEYDOWN:
                if (!g_keys[wp & 0xFF]) g_keysDown[wp & 0xFF] = true;
                g_keys[wp & 0xFF] = true;
                return 0;
            case WM_KEYUP:
                g_keys[wp & 0xFF] = false;
                g_keysUp[wp & 0xFF] = true;
                return 0;
            case WM_LBUTTONDOWN:
                if (!g_keys[1]) g_keysDown[1] = true;
                g_keys[1] = true;
                return 0;
            case WM_LBUTTONUP:
                g_keys[1] = false;
                g_keysUp[1] = true;
                return 0;
            case WM_RBUTTONDOWN:
                if (!g_keys[2]) g_keysDown[2] = true;
                g_keys[2] = true;
                return 0;
            case WM_RBUTTONUP:
                g_keys[2] = false;
                g_keysUp[2] = true;
                return 0;
            case WM_MOUSEMOVE:
                g_mousePos.x = (float)LOWORD(lp);
                g_mousePos.y = (float)HIWORD(lp);
                return 0;
            case WM_SIZE:
                g_width = LOWORD(lp);
                g_height = HIWORD(lp);
#ifndef TITAN_NO_GRAPHICS
                if (g_width > 0 && g_height > 0 && g_hrc) glViewport(0, 0, g_width, g_height);
#endif
                return 0;
        }
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

namespace Window {
    inline bool Create(const char* title, int width, int height) {
        Internal::g_width = width;
        Internal::g_height = height;
        printf("[Logger] Window::Create(\"%s\", %d, %d)\n", title, width, height);
        
        WNDCLASSA wc = {};
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = Internal::WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "TitanWindow";
        RegisterClassA(&wc);
        
        RECT rect = {0, 0, width, height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        
        Internal::g_hwnd = CreateWindowA("TitanWindow", title,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        
        if (!Internal::g_hwnd) return false;
        
        Internal::g_hdc = GetDC(Internal::g_hwnd);
        
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        
        int format = ChoosePixelFormat(Internal::g_hdc, &pfd);
        SetPixelFormat(Internal::g_hdc, format, &pfd);
        
        Internal::g_hrc = wglCreateContext(Internal::g_hdc);
        wglMakeCurrent(Internal::g_hdc, Internal::g_hrc);
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        Internal::g_fontBase = glGenLists(256);
        HFONT font = CreateFontA(
            -18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Consolas");
        HFONT oldFont = (HFONT)SelectObject(Internal::g_hdc, font);
        wglUseFontBitmapsA(Internal::g_hdc, 0, 256, Internal::g_fontBase);
        SelectObject(Internal::g_hdc, oldFont);
        DeleteObject(font);
        
        QueryPerformanceFrequency(&Internal::g_freq);
        QueryPerformanceCounter(&Internal::g_lastTime);
        
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        
        return true;
    }
    
    inline bool IsOpen() {
        memset(Internal::g_keysDown, 0, sizeof(Internal::g_keysDown));
        memset(Internal::g_keysUp, 0, sizeof(Internal::g_keysUp));
        
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        Internal::g_deltaTime = (float)(currentTime.QuadPart - Internal::g_lastTime.QuadPart) / Internal::g_freq.QuadPart;
        Internal::g_lastTime = currentTime;
        Internal::g_totalTime += Internal::g_deltaTime;
        
        static float fpsTimer = 0;
        static int frameCount = 0;
        frameCount++;
        fpsTimer += Internal::g_deltaTime;
        if (fpsTimer >= 1.0f) {
            Internal::g_fps = frameCount;
            frameCount = 0;
            fpsTimer = 0;
        }
        
        if (Internal::g_mouseLocked) {
            POINT mousePos;
            GetCursorPos(&mousePos);
            Internal::g_mouseDelta.x = (float)(mousePos.x - Internal::g_mouseCenterX);
            Internal::g_mouseDelta.y = (float)(mousePos.y - Internal::g_mouseCenterY);
            SetCursorPos(Internal::g_mouseCenterX, Internal::g_mouseCenterY);
        } else {
            Internal::g_mouseDelta = {0, 0};
        }
        
        return Internal::g_running;
    }
    
    inline void SwapBuffers() {
        ::SwapBuffers(Internal::g_hdc);
    }
    
    inline void Close() {
        Internal::g_running = false;
    }
    
    inline int GetWidth() { return Internal::g_width; }
    inline int GetHeight() { return Internal::g_height; }
    
    inline void SetTitle(const char* title) {
        SetWindowTextA(Internal::g_hwnd, title);
    }
}

namespace Input {
    inline void MapAction(const char* name, KeyCode key) {
        Internal::g_actions[name] = key;
    }
    
    inline bool GetAction(const char* name) {
        auto it = Internal::g_actions.find(name);
        if (it == Internal::g_actions.end()) return false;
        return Internal::g_keys[(int)it->second];
    }
    
    inline bool GetActionDown(const char* name) {
        auto it = Internal::g_actions.find(name);
        if (it == Internal::g_actions.end()) return false;
        return Internal::g_keysDown[(int)it->second];
    }
    
    inline bool GetActionUp(const char* name) {
        auto it = Internal::g_actions.find(name);
        if (it == Internal::g_actions.end()) return false;
        return Internal::g_keysUp[(int)it->second];
    }
    
    inline bool GetKey(KeyCode key) {
        return Internal::g_keys[(int)key];
    }
    
    inline bool GetKeyDown(KeyCode key) {
        return Internal::g_keysDown[(int)key];
    }
    
    inline Vec2 GetMousePos() {
        return Internal::g_mousePos;
    }
    
    inline Vec2 GetMouseDelta() {
        return Internal::g_mouseDelta;
    }
    
    inline void SetMouseLocked(bool locked) {
        Internal::g_mouseLocked = locked;
        ShowCursor(!locked);
        if (locked) {
            RECT wr;
            GetWindowRect(Internal::g_hwnd, &wr);
            Internal::g_mouseCenterX = (wr.left + wr.right) / 2;
            Internal::g_mouseCenterY = (wr.top + wr.bottom) / 2;
            SetCursorPos(Internal::g_mouseCenterX, Internal::g_mouseCenterY);
        }
    }
    
    namespace Controls {
        inline bool g_invertFB = false;
        inline bool g_invertLR = false;
        inline bool g_autoDetected = false;
        
        inline void InvertForwardBack(bool invert) { g_invertFB = invert; }
        inline void InvertLeftRight(bool invert) { g_invertLR = invert; }
        inline bool IsForwardBackInverted() { return g_invertFB; }
        inline bool IsLeftRightInverted() { return g_invertLR; }
        inline void SetAutoDetected(bool val) { g_autoDetected = val; }
        inline bool IsAutoDetected() { return g_autoDetected; }
        
        inline void SwapForwardBack() { g_invertFB = !g_invertFB; }
        inline void SwapLeftRight() { g_invertLR = !g_invertLR; }
        
        inline void Reset() { g_invertFB = false; g_invertLR = false; g_autoDetected = false; }
    }
}

namespace Time {
    inline float GetDelta() { return Internal::g_deltaTime; }
    inline float GetTime() { return Internal::g_totalTime; }
    inline int GetFPS() { return Internal::g_fps; }
    inline void Sleep(int ms) { ::Sleep(ms); }
}

namespace Graphics {
    inline void Clear(float r, float g, float b) {
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    inline void SetViewport(int x, int y, int w, int h) {
        glViewport(x, y, w, h);
    }
    
    inline void DrawText(float x, float y, const char* text) {
        glRasterPos2f(x, y + 14);
        glPushAttrib(GL_LIST_BIT);
        glListBase(Internal::g_fontBase);
        glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
        glPopAttrib();
    }
    
    inline void DrawTextCentered(float x, float y, float w, const char* text) {
        float textW = strlen(text) * 10.0f;
        DrawText(x + (w - textW) * 0.5f, y, text);
    }
}

namespace Camera3D {
    inline void Create() {}
    
    inline void SetPosition(float x, float y, float z) {
        Internal::g_camPos = {x, y, z};
    }
    
    inline void SetRotation(float pitch, float yaw) {
        Internal::g_camPitch = pitch;
        Internal::g_camYaw = yaw;
    }
    
    inline void SetFOV(float fov) {
        Internal::g_camFOV = fov;
    }
    
    inline void SetSensitivity(float sens) {
        Internal::g_camSens = sens;
    }
    
    inline void SetSpeed(float speed) {
        Internal::g_camSpeed = speed;
    }
    
    inline Vec3 GetPosition() {
        return Internal::g_camPos;
    }
    
    inline Vec3 GetForward() {
        return Vec3{
            sinf(Internal::g_camYaw) * cosf(Internal::g_camPitch),
            -sinf(Internal::g_camPitch),
            -cosf(Internal::g_camYaw) * cosf(Internal::g_camPitch)
        }.Normalized();
    }
    
    inline Vec3 GetRight() {
        return Vec3{cosf(Internal::g_camYaw), 0, sinf(Internal::g_camYaw)}.Normalized();
    }
    
    inline void Rotate(float dx, float dy) {
        Internal::g_camYaw += dx * Internal::g_camSens;
        Internal::g_camPitch += dy * Internal::g_camSens;
        if (Internal::g_camPitch > 1.5f) Internal::g_camPitch = 1.5f;
        if (Internal::g_camPitch < -1.5f) Internal::g_camPitch = -1.5f;
    }
    
    inline void Move(float forward, float right, float up, float dt) {
        Vec3 fwd = GetForward();
        Vec3 rgt = GetRight();
        fwd.y = 0; fwd = fwd.Normalized();
        rgt.y = 0; rgt = rgt.Normalized();
        Internal::g_camPos = Internal::g_camPos + fwd * forward * Internal::g_camSpeed * dt;
        Internal::g_camPos = Internal::g_camPos + rgt * right * Internal::g_camSpeed * dt;
        Internal::g_camPos.y += up * Internal::g_camSpeed * dt;
    }
    
    inline Mat4 GetViewMatrix() {
        Vec3 target = Internal::g_camPos + GetForward();
        return Mat4::LookAt(Internal::g_camPos, target, {0, 1, 0});
    }
    
    inline Mat4 GetProjectionMatrix() {
        float aspect = (float)Internal::g_width / Internal::g_height;
        return Mat4::Perspective(Internal::g_camFOV, aspect, 0.1f, 1000.0f);
    }
    
    inline void Apply() {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(GetProjectionMatrix().m);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(GetViewMatrix().m);
    }
}

namespace Light {
    inline void Enable() {
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    }
    
    inline void Disable() {
        glDisable(GL_LIGHTING);
    }
    
    inline void AddDirectional(float dx, float dy, float dz, float r, float g, float b, float intensity) {
        glEnable(GL_LIGHT0);
        float pos[] = {-dx, -dy, -dz, 0};
        float color[] = {r * intensity, g * intensity, b * intensity, 1};
        float ambient[] = {0.1f, 0.1f, 0.1f, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    }
    
    inline void AddPoint(float x, float y, float z, float r, float g, float b, float intensity, float range, int index = 1) {
        glEnable(GL_LIGHT0 + index);
        float pos[] = {x, y, z, 1};
        float color[] = {r * intensity, g * intensity, b * intensity, 1};
        glLightfv(GL_LIGHT0 + index, GL_POSITION, pos);
        glLightfv(GL_LIGHT0 + index, GL_DIFFUSE, color);
        glLightf(GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0 + index, GL_LINEAR_ATTENUATION, 2.0f / range);
        glLightf(GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, 1.0f / (range * range));
    }
    
    inline void Clear() {
        for (int i = 0; i < 8; i++) glDisable(GL_LIGHT0 + i);
    }
}

namespace Collision {
    struct AABB {
        Vec3 min, max;
        
        bool Contains(const Vec3& p) const {
            return p.x >= min.x && p.x <= max.x &&
                   p.y >= min.y && p.y <= max.y &&
                   p.z >= min.z && p.z <= max.z;
        }
        
        bool Intersects(const AABB& o) const {
            return min.x <= o.max.x && max.x >= o.min.x &&
                   min.y <= o.max.y && max.y >= o.min.y &&
                   min.z <= o.max.z && max.z >= o.min.z;
        }
    };
    
    inline std::vector<AABB> g_staticBoxes;
    inline std::vector<AABB> g_dynamicBoxes;
    
    inline void AddStaticBox(const Vec3& pos, float size) {
        float s = size * 0.5f;
        g_staticBoxes.push_back({{pos.x - s, pos.y - s, pos.z - s}, {pos.x + s, pos.y + s, pos.z + s}});
    }
    
    inline void AddStaticBox(const Vec3& min, const Vec3& max) {
        g_staticBoxes.push_back({min, max});
    }
    
    inline void ClearStatic() { g_staticBoxes.clear(); }
    inline void ClearDynamic() { g_dynamicBoxes.clear(); }
    
    inline bool CheckCollision(const Vec3& pos, float radius) {
        AABB player = {
            {pos.x - radius, pos.y - radius, pos.z - radius},
            {pos.x + radius, pos.y + radius, pos.z + radius}
        };
        for (const auto& box : g_staticBoxes) {
            if (player.Intersects(box)) return true;
        }
        return false;
    }
    
    inline Vec3 ResolveCollision(const Vec3& oldPos, const Vec3& newPos, float radius) {
        Vec3 result = newPos;
        
        Vec3 testX = {newPos.x, oldPos.y, oldPos.z};
        if (CheckCollision(testX, radius)) result.x = oldPos.x;
        
        Vec3 testY = {result.x, newPos.y, oldPos.z};
        if (CheckCollision(testY, radius)) result.y = oldPos.y;
        
        Vec3 testZ = {result.x, result.y, newPos.z};
        if (CheckCollision(testZ, radius)) result.z = oldPos.z;
        
        return result;
    }
    
    inline bool Raycast(const Vec3& origin, const Vec3& dir, float maxDist, Vec3* hitPoint = nullptr) {
        for (const auto& box : g_staticBoxes) {
            float tmin = 0, tmax = maxDist;
            
            for (int i = 0; i < 3; i++) {
                float o = (&origin.x)[i];
                float d = (&dir.x)[i];
                float bmin = (&box.min.x)[i];
                float bmax = (&box.max.x)[i];
                
                if (fabsf(d) < 0.0001f) {
                    if (o < bmin || o > bmax) { tmin = maxDist + 1; break; }
                } else {
                    float t1 = (bmin - o) / d;
                    float t2 = (bmax - o) / d;
                    if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
                    if (t1 > tmin) tmin = t1;
                    if (t2 < tmax) tmax = t2;
                    if (tmin > tmax) { tmin = maxDist + 1; break; }
                }
            }
            
            if (tmin <= maxDist) {
                if (hitPoint) *hitPoint = origin + dir * tmin;
                return true;
            }
        }
        return false;
    }
}

namespace MainMenu {
    enum class State { Main, Singleplayer, Multiplayer, ServerBrowser, HostGame, Settings, Quit };
    
    struct ServerInfo {
        char name[64];
        char ip[32];
        uint16_t port;
        int players;
        int maxPlayers;
        int ping;
        bool online;
    };
    
    inline State g_state = State::Main;
    inline int g_selected = 0;
    inline std::vector<ServerInfo> g_servers;
    inline char g_playerName[32] = "Player";
    inline char g_serverIP[64] = "127.0.0.1";
    inline char g_serverName[64] = "My Server";
    inline uint16_t g_hostPort = 27015;
    inline int g_maxPlayers = 16;
    inline bool g_isHost = false;
    inline float g_mouseSens = 1.0f;
    inline float g_volume = 1.0f;
    inline bool g_fullscreen = false;
    inline bool g_vsync = true;
    inline std::function<void()> g_onSingleplayer = nullptr;
    inline std::function<void(const char* ip, uint16_t port)> g_onJoinServer = nullptr;
    inline std::function<void(const char* name)> g_onHostServer = nullptr;
    
    inline void SetOnSingleplayer(std::function<void()> cb) { g_onSingleplayer = cb; }
    inline void SetOnJoinServer(std::function<void(const char*, uint16_t)> cb) { g_onJoinServer = cb; }
    inline void SetOnHostServer(std::function<void(const char*)> cb) { g_onHostServer = cb; }
    
    inline void AddServer(const char* name, const char* ip, uint16_t port, int players, int maxPlayers, int ping) {
        ServerInfo srv = {};
        strncpy(srv.name, name, 63);
        strncpy(srv.ip, ip, 31);
        srv.port = port;
        srv.players = players;
        srv.maxPlayers = maxPlayers;
        srv.ping = ping;
        srv.online = true;
        g_servers.push_back(srv);
    }
    
    inline void ClearServers() { g_servers.clear(); }
    
    inline void DrawGradientRect(float x, float y, float w, float h, 
                                  float r1, float g1, float b1, float a1,
                                  float r2, float g2, float b2, float a2) {
        glBegin(GL_QUADS);
        glColor4f(r1, g1, b1, a1);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glColor4f(r2, g2, b2, a2);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    inline void DrawButton(float x, float y, float w, float h, const char* text, bool selected, bool hovered) {
        if (selected) {
            glColor4f(0.2f, 0.6f, 0.9f, 0.9f);
        } else if (hovered) {
            glColor4f(0.3f, 0.3f, 0.4f, 0.8f);
        } else {
            glColor4f(0.15f, 0.15f, 0.2f, 0.8f);
        }
        
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
        
        if (selected) {
            glColor4f(0.4f, 0.8f, 1.0f, 1.0f);
        } else {
            glColor4f(0.5f, 0.5f, 0.6f, 1.0f);
        }
        glLineWidth(selected ? 3.0f : 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
        
        if (selected) {
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            glColor3f(0.8f, 0.8f, 0.8f);
        }
        
        float textW = strlen(text) * 10.0f;
        float tx = x + (w - textW) * 0.5f;
        float ty = y + (h - 14) * 0.5f;
        Graphics::DrawText(tx, ty, text);
    }
    
    inline void DrawTitle(const char* text, float y) {
        float w = (float)Internal::g_width;
        float textW = strlen(text) * 18.0f;
        float x = (w - textW) * 0.5f;
        
        glColor4f(0.1f, 0.5f, 0.9f, 0.5f);
        Graphics::DrawText(x + 2, y + 2, text);
        
        glColor3f(1.0f, 1.0f, 1.0f);
        Graphics::DrawText(x, y, text);
    }
    
    inline void DrawServerList(float x, float y, float w, float h) {
        glColor4f(0.05f, 0.05f, 0.1f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
        
        glColor4f(0.3f, 0.5f, 0.7f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();
        
        float rowH = 30.0f;
        float py = y + 15;
        
        glColor3f(0.6f, 0.6f, 0.7f);
        Graphics::DrawText(x + 25, py, "SERVER NAME");
        Graphics::DrawText(x + w * 0.55f, py, "PLAYERS");
        Graphics::DrawText(x + w * 0.8f, py, "PING");
        
        py += 25;
        glColor4f(0.3f, 0.3f, 0.4f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(x + 5, py); glVertex2f(x + w - 5, py);
        glEnd();
        py += 10;
        
        for (size_t i = 0; i < g_servers.size() && py < y + h - 20; i++) {
            bool sel = (int)i == g_selected;
            
            if (sel) {
                glColor4f(0.2f, 0.4f, 0.6f, 0.5f);
                glBegin(GL_QUADS);
                glVertex2f(x + 5, py - 2);
                glVertex2f(x + w - 5, py - 2);
                glVertex2f(x + w - 5, py + rowH - 2);
                glVertex2f(x + 5, py + rowH - 2);
                glEnd();
            }
            
            if (g_servers[i].online) {
                glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
            } else {
                glColor4f(1.0f, 0.3f, 0.3f, 1.0f);
            }
            glBegin(GL_QUADS);
            glVertex2f(x + 10, py + 5);
            glVertex2f(x + 20, py + 5);
            glVertex2f(x + 20, py + 15);
            glVertex2f(x + 10, py + 15);
            glEnd();
            
            glColor3f(sel ? 1.0f : 0.8f, sel ? 1.0f : 0.8f, sel ? 1.0f : 0.8f);
            Graphics::DrawText(x + 25, py, g_servers[i].name);
            
            char playerStr[32];
            snprintf(playerStr, 32, "%d/%d", g_servers[i].players, g_servers[i].maxPlayers);
            Graphics::DrawText(x + w * 0.55f, py, playerStr);
            
            char pingStr[16];
            snprintf(pingStr, 16, "%dms", g_servers[i].ping);
            if (g_servers[i].ping < 50) glColor3f(0.3f, 1.0f, 0.3f);
            else if (g_servers[i].ping < 100) glColor3f(1.0f, 1.0f, 0.3f);
            else glColor3f(1.0f, 0.3f, 0.3f);
            Graphics::DrawText(x + w * 0.8f, py, pingStr);
            
            py += rowH;
        }
    }
    
    inline bool Update() {
        int btnCount = 0;
        const char* buttons[10];
        
        switch (g_state) {
            case State::Main:
                buttons[0] = "SINGLEPLAYER";
                buttons[1] = "MULTIPLAYER";
                buttons[2] = "SETTINGS";
                buttons[3] = "QUIT";
                btnCount = 4;
                break;
            case State::Multiplayer:
                buttons[0] = "SERVER BROWSER";
                buttons[1] = "HOST GAME";
                buttons[2] = "DIRECT CONNECT";
                buttons[3] = "BACK";
                btnCount = 4;
                break;
            case State::ServerBrowser:
                btnCount = (int)g_servers.size() + 2;
                break;
            case State::HostGame:
                btnCount = 3;
                break;
            case State::Settings:
                btnCount = 5;
                break;
            default:
                break;
        }
        
        if (Input::GetKeyDown(KeyCode::Up)) {
            g_selected--;
            if (g_selected < 0) g_selected = btnCount - 1;
        }
        if (Input::GetKeyDown(KeyCode::Down)) {
            g_selected++;
            if (g_selected >= btnCount) g_selected = 0;
        }
        
        if (Input::GetKeyDown(KeyCode::Enter) || Input::GetKeyDown(KeyCode::Space)) {
            switch (g_state) {
                case State::Main:
                    if (g_selected == 0) {
                        if (g_onSingleplayer) g_onSingleplayer();
                        return false;
                    }
                    else if (g_selected == 1) { g_state = State::Multiplayer; g_selected = 0; }
                    else if (g_selected == 2) { g_state = State::Settings; g_selected = 0; }
                    else if (g_selected == 3) { return false; }
                    break;
                case State::Multiplayer:
                    if (g_selected == 0) { g_state = State::ServerBrowser; g_selected = 0; }
                    else if (g_selected == 1) { g_state = State::HostGame; g_selected = 0; }
                    else if (g_selected == 2) {
                        if (g_onJoinServer) g_onJoinServer(g_serverIP, 27015);
                        return false;
                    }
                    else if (g_selected == 3) { g_state = State::Main; g_selected = 0; }
                    break;
                case State::ServerBrowser:
                    if (g_selected < (int)g_servers.size()) {
                        if (g_onJoinServer) g_onJoinServer(g_servers[g_selected].ip, g_servers[g_selected].port);
                        return false;
                    }
                    else if (g_selected == (int)g_servers.size()) { /* Refresh */ }
                    else { g_state = State::Multiplayer; g_selected = 0; }
                    break;
                case State::HostGame:
                    if (g_selected == 0) {
                        g_isHost = true;
                        if (g_onHostServer) g_onHostServer(g_serverName);
                        return false;
                    }
                    else if (g_selected == 1) { g_maxPlayers = (g_maxPlayers % 32) + 2; }
                    else { g_state = State::Multiplayer; g_selected = 0; }
                    break;
                case State::Settings:
                    if (g_selected == 0) { g_mouseSens = fmodf(g_mouseSens + 0.25f, 2.5f) + 0.25f; }
                    else if (g_selected == 1) { g_volume = fmodf(g_volume + 0.1f, 1.1f); }
                    else if (g_selected == 2) { g_fullscreen = !g_fullscreen; }
                    else if (g_selected == 3) { g_vsync = !g_vsync; }
                    else { g_state = State::Main; g_selected = 0; }
                    break;
                default:
                    break;
            }
        }
        
        if (Input::GetKeyDown(KeyCode::Escape)) {
            if (g_state == State::Main) return false;
            g_state = State::Main;
            g_selected = 0;
        }
        
        return true;
    }
    
    inline void Render() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float w = (float)Internal::g_width;
        float h = (float)Internal::g_height;
        
        DrawGradientRect(0, 0, w, h, 0.02f, 0.02f, 0.08f, 1.0f, 0.08f, 0.05f, 0.15f, 1.0f);
        
        for (int i = 0; i < 50; i++) {
            float px = fmodf(i * 137.5f + Internal::g_totalTime * 5, w);
            float py = fmodf(i * 89.3f + Internal::g_totalTime * 3, h);
            float size = 1.0f + sinf(i + Internal::g_totalTime) * 0.5f;
            float alpha = 0.3f + sinf(i * 0.5f + Internal::g_totalTime * 2) * 0.2f;
            glColor4f(0.3f, 0.5f, 0.8f, alpha);
            glBegin(GL_QUADS);
            glVertex2f(px, py); glVertex2f(px + size, py);
            glVertex2f(px + size, py + size); glVertex2f(px, py + size);
            glEnd();
        }
        
        float btnW = 300;
        float btnH = 50;
        float btnX = (w - btnW) * 0.5f;
        float btnY = 120;
        float gap = 15;
        
        switch (g_state) {
            case State::Main:
                DrawButton(btnX, btnY, btnW, btnH, "SINGLEPLAYER", g_selected == 0, false);
                DrawButton(btnX, btnY + btnH + gap, btnW, btnH, "MULTIPLAYER", g_selected == 1, false);
                DrawButton(btnX, btnY + (btnH + gap) * 2, btnW, btnH, "SETTINGS", g_selected == 2, false);
                DrawButton(btnX, btnY + (btnH + gap) * 3, btnW, btnH, "QUIT", g_selected == 3, false);
                break;
                
            case State::Multiplayer:
                DrawButton(btnX, btnY, btnW, btnH, "SERVER BROWSER", g_selected == 0, false);
                DrawButton(btnX, btnY + btnH + gap, btnW, btnH, "HOST GAME", g_selected == 1, false);
                DrawButton(btnX, btnY + (btnH + gap) * 2, btnW, btnH, "DIRECT CONNECT", g_selected == 2, false);
                DrawButton(btnX, btnY + (btnH + gap) * 3, btnW, btnH, "BACK", g_selected == 3, false);
                break;
                
            case State::ServerBrowser:
                DrawServerList(50, 130, w - 100, h - 250);
                DrawButton(w * 0.5f - 160, h - 100, 150, 45, "REFRESH", g_selected == (int)g_servers.size(), false);
                DrawButton(w * 0.5f + 10, h - 100, 150, 45, "BACK", g_selected == (int)g_servers.size() + 1, false);
                break;
            
            case State::HostGame: {
                glColor3f(1.0f, 1.0f, 1.0f);
                Graphics::DrawText(btnX, btnY - 40, "HOST A SERVER");
                
                char serverLabel[128];
                snprintf(serverLabel, 128, "Server Name: %s", g_serverName);
                glColor3f(0.7f, 0.7f, 0.8f);
                Graphics::DrawText(btnX, btnY - 10, serverLabel);
                
                DrawButton(btnX, btnY + 30, btnW, btnH, "START SERVER", g_selected == 0, false);
                
                char maxPlayersStr[64];
                snprintf(maxPlayersStr, 64, "Max Players: %d", g_maxPlayers);
                DrawButton(btnX, btnY + 30 + btnH + gap, btnW, btnH, maxPlayersStr, g_selected == 1, false);
                
                DrawButton(btnX, btnY + 30 + (btnH + gap) * 2, btnW, btnH, "BACK", g_selected == 2, false);
                break;
            }
            
            case State::Settings: {
                glColor3f(1.0f, 1.0f, 1.0f);
                Graphics::DrawText(btnX, btnY - 40, "SETTINGS");
                
                char sensStr[64];
                snprintf(sensStr, 64, "Mouse Sensitivity: %.2f", g_mouseSens);
                DrawButton(btnX, btnY, btnW, btnH, sensStr, g_selected == 0, false);
                
                char volStr[64];
                snprintf(volStr, 64, "Volume: %.0f%%", g_volume * 100);
                DrawButton(btnX, btnY + btnH + gap, btnW, btnH, volStr, g_selected == 1, false);
                
                DrawButton(btnX, btnY + (btnH + gap) * 2, btnW, btnH, 
                    g_fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF", g_selected == 2, false);
                    
                DrawButton(btnX, btnY + (btnH + gap) * 3, btnW, btnH,
                    g_vsync ? "VSync: ON" : "VSync: OFF", g_selected == 3, false);
                
                DrawButton(btnX, btnY + (btnH + gap) * 4, btnW, btnH, "BACK", g_selected == 4, false);
                break;
            }
                
            default:
                break;
        }
        
        glColor3f(0.5f, 0.5f, 0.6f);
        Graphics::DrawText(20, h - 35, "Use UP/DOWN to navigate, ENTER to select, ESC to go back");
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    inline void Reset() {
        g_state = State::Main;
        g_selected = 0;
    }
}

namespace Mesh {
    inline void DrawCube(float x, float y, float z, float size, float r, float g, float b) {
        float s = size * 0.5f;
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(x-s, y-s, z+s); glVertex3f(x+s, y-s, z+s); glVertex3f(x+s, y+s, z+s); glVertex3f(x-s, y+s, z+s);
        glNormal3f(0, 0, -1);
        glVertex3f(x+s, y-s, z-s); glVertex3f(x-s, y-s, z-s); glVertex3f(x-s, y+s, z-s); glVertex3f(x+s, y+s, z-s);
        glNormal3f(0, 1, 0);
        glVertex3f(x-s, y+s, z+s); glVertex3f(x+s, y+s, z+s); glVertex3f(x+s, y+s, z-s); glVertex3f(x-s, y+s, z-s);
        glNormal3f(0, -1, 0);
        glVertex3f(x-s, y-s, z-s); glVertex3f(x+s, y-s, z-s); glVertex3f(x+s, y-s, z+s); glVertex3f(x-s, y-s, z+s);
        glNormal3f(1, 0, 0);
        glVertex3f(x+s, y-s, z+s); glVertex3f(x+s, y-s, z-s); glVertex3f(x+s, y+s, z-s); glVertex3f(x+s, y+s, z+s);
        glNormal3f(-1, 0, 0);
        glVertex3f(x-s, y-s, z-s); glVertex3f(x-s, y-s, z+s); glVertex3f(x-s, y+s, z+s); glVertex3f(x-s, y+s, z-s);
        glEnd();
    }
    
    inline void DrawSphere(float x, float y, float z, float radius, float r, float g, float b) {
        glColor3f(r, g, b);
        const int seg = 16, ring = 12;
        for (int i = 0; i < ring; i++) {
            float phi1 = 3.14159f * i / ring;
            float phi2 = 3.14159f * (i + 1) / ring;
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= seg; j++) {
                float theta = 6.28318f * j / seg;
                float x1 = sinf(phi1) * cosf(theta), y1 = cosf(phi1), z1 = sinf(phi1) * sinf(theta);
                float x2 = sinf(phi2) * cosf(theta), y2 = cosf(phi2), z2 = sinf(phi2) * sinf(theta);
                glNormal3f(x1, y1, z1);
                glVertex3f(x + x1 * radius, y + y1 * radius, z + z1 * radius);
                glNormal3f(x2, y2, z2);
                glVertex3f(x + x2 * radius, y + y2 * radius, z + z2 * radius);
            }
            glEnd();
        }
    }
    
    inline void DrawPlane(float size, int divisions, float r, float g, float b) {
        float half = size * 0.5f;
        float step = size / divisions;
        glBegin(GL_QUADS);
        for (int i = 0; i < divisions; i++) {
            for (int j = 0; j < divisions; j++) {
                float px = -half + i * step;
                float pz = -half + j * step;
                if ((i + j) % 2 == 0) glColor3f(r, g, b);
                else glColor3f(r * 0.7f, g * 0.7f, b * 0.7f);
                glNormal3f(0, 1, 0);
                glVertex3f(px, 0, pz);
                glVertex3f(px + step, 0, pz);
                glVertex3f(px + step, 0, pz + step);
                glVertex3f(px, 0, pz + step);
            }
        }
        glEnd();
    }
}

namespace UI {
    struct MenuButton {
        std::string label;
        std::function<void()> callback;
    };
    
    inline bool g_pauseOpen = false;
    inline int g_pauseSelected = 0;
    inline std::vector<MenuButton> g_pauseButtons;
    inline KeyCode g_pauseKey = KeyCode::Escape;
    inline std::string g_pauseTitle = "PAUSED";
    inline float g_btnWidth = 200.0f;
    inline float g_btnHeight = 40.0f;
    
    inline void SetPauseKey(KeyCode key) {
        g_pauseKey = key;
    }
    
    inline void SetPauseTitle(const char* title) {
        g_pauseTitle = title;
    }
    
    inline void SetPauseButtonSize(float w, float h) {
        g_btnWidth = w;
        g_btnHeight = h;
    }
    
    inline void ClearPauseButtons() {
        g_pauseButtons.clear();
    }
    
    inline void AddPauseButton(const char* label, std::function<void()> callback) {
        g_pauseButtons.push_back({label, callback});
    }
    
    inline void OpenPause() {
        g_pauseOpen = true;
        g_pauseSelected = 0;
        Input::SetMouseLocked(false);
    }
    
    inline void ClosePause() {
        g_pauseOpen = false;
        Input::SetMouseLocked(true);
    }
    
    inline void TogglePause() {
        if (g_pauseOpen) ClosePause();
        else OpenPause();
    }
    
    inline bool IsPaused() {
        return g_pauseOpen;
    }
    
    inline void SetPauseCallbacks(std::function<void()> onResume, std::function<void()> onSettings, std::function<void()> onQuit) {
        g_pauseButtons.clear();
        g_pauseButtons.push_back({"RESUME", onResume ? onResume : []() { ClosePause(); }});
        g_pauseButtons.push_back({"SETTINGS", onSettings});
        g_pauseButtons.push_back({"QUIT", onQuit ? onQuit : []() { Window::Close(); }});
    }
    
    inline void DrawPauseMenu() {
        if (!g_pauseOpen || g_pauseButtons.empty()) return;
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0, 0, 0, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f((float)Internal::g_width, 0);
        glVertex2f((float)Internal::g_width, (float)Internal::g_height);
        glVertex2f(0, (float)Internal::g_height);
        glEnd();
        glDisable(GL_BLEND);
        
        float cx = Internal::g_width * 0.5f;
        float cy = Internal::g_height * 0.5f;
        float gap = 10;
        int btnCount = (int)g_pauseButtons.size();
        
        float startY = cy - (btnCount * g_btnHeight + (btnCount - 1) * gap) * 0.5f;
        
        Vec2 mouse = Input::GetMousePos();
        
        for (int i = 0; i < btnCount; i++) {
            float bx = cx - g_btnWidth * 0.5f;
            float by = startY + i * (g_btnHeight + gap);
            
            bool hover = mouse.x >= bx && mouse.x <= bx + g_btnWidth && mouse.y >= by && mouse.y <= by + g_btnHeight;
            
            if (hover) {
                g_pauseSelected = i;
                glColor3f(0.3f, 0.5f, 0.8f);
            } else if (i == g_pauseSelected) {
                glColor3f(0.2f, 0.4f, 0.7f);
            } else {
                glColor3f(0.15f, 0.15f, 0.2f);
            }
            
            glBegin(GL_QUADS);
            glVertex2f(bx, by);
            glVertex2f(bx + g_btnWidth, by);
            glVertex2f(bx + g_btnWidth, by + g_btnHeight);
            glVertex2f(bx, by + g_btnHeight);
            glEnd();
            
            glColor3f(1, 1, 1);
            glLineWidth(2);
            glBegin(GL_LINE_LOOP);
            glVertex2f(bx, by);
            glVertex2f(bx + g_btnWidth, by);
            glVertex2f(bx + g_btnWidth, by + g_btnHeight);
            glVertex2f(bx, by + g_btnHeight);
            glEnd();
            
            if (hover && Input::GetKeyDown(KeyCode::MouseLeft)) {
                if (g_pauseButtons[i].callback) g_pauseButtons[i].callback();
            }
        }
        
        if (Input::GetKeyDown(KeyCode::Up)) {
            g_pauseSelected--;
            if (g_pauseSelected < 0) g_pauseSelected = btnCount - 1;
        }
        if (Input::GetKeyDown(KeyCode::Down)) {
            g_pauseSelected++;
            if (g_pauseSelected >= btnCount) g_pauseSelected = 0;
        }
        if (Input::GetKeyDown(KeyCode::Enter)) {
            if (g_pauseSelected >= 0 && g_pauseSelected < btnCount && g_pauseButtons[g_pauseSelected].callback) {
                g_pauseButtons[g_pauseSelected].callback();
            }
        }
        
        glColor3f(1, 1, 1);
        float titleY = startY - 60;
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2f(cx - 80, titleY + 20);
        glVertex2f(cx + 80, titleY + 20);
        glEnd();
        
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

namespace Debug {
    inline bool g_overlayOpen = false;
    inline std::unordered_map<std::string, int> g_debugValues;
    inline std::unordered_map<std::string, std::string> g_debugStrings;
    
    inline void Log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        printf("[LOG] ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    }
    
    inline void Warning(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        printf("[WARN] ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    }
    
    inline void Error(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        printf("[ERROR] ");
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    }
    
    inline void OpenOverlay() { g_overlayOpen = true; }
    inline void CloseOverlay() { g_overlayOpen = false; }
    inline void ToggleOverlay() { g_overlayOpen = !g_overlayOpen; }
    inline bool IsOverlayOpen() { return g_overlayOpen; }
    
    inline void SetValue(const char* name, int value) {
        g_debugValues[name] = value;
    }
    
    inline void SetValue(const char* name, float value) {
        g_debugValues[name] = (int)value;
    }
    
    inline void SetValue(const char* name, const char* value) {
        g_debugStrings[name] = value;
    }
    
    inline void ClearValues() {
        g_debugValues.clear();
        g_debugStrings.clear();
    }
    
    inline void DrawOverlay() {
        if (!g_overlayOpen) return;
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float panelW = 200;
        float lineH = 18;
        int totalLines = 3 + (int)g_debugValues.size() + (int)g_debugStrings.size();
        float panelH = totalLines * lineH + 20;
        
        glColor4f(0, 0, 0, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(10, 10);
        glVertex2f(10 + panelW, 10);
        glVertex2f(10 + panelW, 10 + panelH);
        glVertex2f(10, 10 + panelH);
        glEnd();
        
        glColor3f(0, 1, 0);
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        glVertex2f(10, 10);
        glVertex2f(10 + panelW, 10);
        glVertex2f(10 + panelW, 10 + panelH);
        glVertex2f(10, 10 + panelH);
        glEnd();
        
        float y = 25;
        
        glBegin(GL_LINES);
        glVertex2f(15, y + 5);
        glVertex2f(15 + panelW - 10, y + 5);
        glEnd();
        y += lineH;
        
        glColor3f(0.5f, 1, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(15, y); glVertex2f(25, y); glVertex2f(25, y + 10); glVertex2f(15, y + 10);
        glEnd();
        y += lineH;
        
        glColor3f(1, 1, 0);
        glBegin(GL_QUADS);
        glVertex2f(15, y); glVertex2f(25, y); glVertex2f(25, y + 10); glVertex2f(15, y + 10);
        glEnd();
        y += lineH;
        
        for (auto& kv : g_debugValues) {
            glColor3f(0, 0.8f, 1);
            glBegin(GL_QUADS);
            float barW = 50.0f * (float)(kv.second % 100) / 100.0f;
            glVertex2f(15, y); glVertex2f(15 + barW, y); glVertex2f(15 + barW, y + 10); glVertex2f(15, y + 10);
            glEnd();
            y += lineH;
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    inline void DrawCrosshair(float size = 10.0f) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0, 1, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        float cx = Internal::g_width * 0.5f;
        float cy = Internal::g_height * 0.5f;
        glVertex2f(cx - size, cy);
        glVertex2f(cx + size, cy);
        glVertex2f(cx, cy - size);
        glVertex2f(cx, cy + size);
        glEnd();
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
}

namespace Logger {
    inline constexpr int MAX_LINES = 20;
    inline constexpr float FONT_SIZE = 12.0f;
    inline constexpr float LINE_HEIGHT = 14.0f;
    inline constexpr float MARGIN = 10.0f;
    
    struct LogEntry {
        char text[256];
        float time;
        uint8_t r, g, b;
    };
    
    inline LogEntry g_logs[MAX_LINES] = {};
    inline int g_logCount = 0;
    inline bool g_enabled = true;
    inline float g_fadeTime = 5.0f;
    
    inline void Enable(bool enabled) { g_enabled = enabled; }
    inline void SetFadeTime(float seconds) { g_fadeTime = seconds; }
    
    inline void Add(const char* msg, uint8_t r = 255, uint8_t g = 230, uint8_t b = 100) {
        if (g_logCount >= MAX_LINES) {
            for (int i = 0; i < MAX_LINES - 1; i++) {
                g_logs[i] = g_logs[i + 1];
            }
            g_logCount = MAX_LINES - 1;
        }
        
        strncpy(g_logs[g_logCount].text, msg, 255);
        g_logs[g_logCount].text[255] = '\0';
        g_logs[g_logCount].time = Internal::g_totalTime;
        g_logs[g_logCount].r = r;
        g_logs[g_logCount].g = g;
        g_logs[g_logCount].b = b;
        g_logCount++;
    }
    
    inline void AddF(const char* fmt, ...) {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 256, fmt, args);
        va_end(args);
        Add(buf);
    }
    
    inline void API(const char* func) {
        char buf[256];
        snprintf(buf, 256, "[API] %s", func);
        Add(buf, 200, 255, 150);
    }
    
    inline void Info(const char* msg) {
        char buf[256];
        snprintf(buf, 256, "[INFO] %s", msg);
        Add(buf, 150, 200, 255);
    }
    
    inline void Warn(const char* msg) {
        char buf[256];
        snprintf(buf, 256, "[WARN] %s", msg);
        Add(buf, 255, 200, 100);
    }
    
    inline void Error(const char* msg) {
        char buf[256];
        snprintf(buf, 256, "[ERROR] %s", msg);
        Add(buf, 255, 100, 100);
    }
    
    inline void Clear() { g_logCount = 0; }
    
    inline void DrawChar(float x, float y, char c, float scale) {
        if (c < 32 || c > 126) return;
        float w = 6 * scale;
        float h = 10 * scale;
        
        static const char* font[95] = {
            "","!","\"|","###","$$$","%0%","&&&","'","(",")",
            "*","+++",",","-",".","/","000","111","222","333",
            "444","555","666","777","888","999","::",";;","<","===",
            ">","???","@@@","AAA","BBB","CCC","DDD","EEE","FFF","GGG",
            "HHH","III","JJJ","KKK","LLL","MMM","NNN","OOO","PPP","QQQ",
            "RRR","SSS","TTT","UUU","VVV","WWW","XXX","YYY","ZZZ","[",
            "\\","]","^^^","___","`","aaa","bbb","ccc","ddd","eee",
            "fff","ggg","hhh","iii","jjj","kkk","lll","mmm","nnn","ooo",
            "ppp","qqq","rrr","sss","ttt","uuu","vvv","www","xxx","yyy",
            "zzz","{","|","}","~"
        };
        
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    inline void DrawText(float x, float y, const char* text, float r, float g, float b, float alpha) {
        glColor4f(r, g, b, alpha);
        float cx = x;
        for (const char* p = text; *p; p++) {
            if (*p >= 32 && *p <= 126) {
                glBegin(GL_QUADS);
                glVertex2f(cx, y);
                glVertex2f(cx + 5, y);
                glVertex2f(cx + 5, y + 8);
                glVertex2f(cx, y + 8);
                glEnd();
            }
            cx += 7;
        }
    }
    
    inline void Render() {
        if (!g_enabled || g_logCount == 0) return;
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float y = MARGIN;
        
        for (int i = 0; i < g_logCount; i++) {
            float age = Internal::g_totalTime - g_logs[i].time;
            float alpha = 1.0f;
            if (age > g_fadeTime - 1.0f) {
                alpha = g_fadeTime - age;
                if (alpha < 0) alpha = 0;
            }
            
            float r = g_logs[i].r / 255.0f;
            float g = g_logs[i].g / 255.0f;
            float b = g_logs[i].b / 255.0f;
            
            glColor4f(0, 0, 0, alpha * 0.6f);
            float textLen = strlen(g_logs[i].text) * 9.0f + 6;
            glBegin(GL_QUADS);
            glVertex2f(MARGIN - 3, y - 2);
            glVertex2f(MARGIN + textLen, y - 2);
            glVertex2f(MARGIN + textLen, y + LINE_HEIGHT + 2);
            glVertex2f(MARGIN - 3, y + LINE_HEIGHT + 2);
            glEnd();
            
            glColor4f(r, g, b, alpha);
            Graphics::DrawText(MARGIN, y, g_logs[i].text);
            
            y += LINE_HEIGHT + 4;
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    inline void Update() {
        int newCount = 0;
        for (int i = 0; i < g_logCount; i++) {
            float age = Internal::g_totalTime - g_logs[i].time;
            if (age < g_fadeTime) {
                if (newCount != i) g_logs[newCount] = g_logs[i];
                newCount++;
            }
        }
        g_logCount = newCount;
    }
}

enum class PacketType : uint8_t {
    None = 0,
    Connect,
    Disconnect,
    Ping,
    Pong,
    PlayerInput,
    WorldState,
    EntitySpawn,
    EntityDestroy,
    EntityUpdate,
    Chat,
    Custom
};

struct Packet {
    PacketType type = PacketType::None;
    ClientID sender = INVALID_CLIENT;
    uint32_t sequence = 0;
    uint32_t ack = 0;
    uint16_t size = 0;
    uint8_t data[1400];
    
    void Write(const void* src, uint16_t len) {
        if (size + len <= 1400) {
            memcpy(data + size, src, len);
            size += len;
        }
    }
    
    template<typename T>
    void Write(const T& val) {
        Write(&val, sizeof(T));
    }
    
    void WriteString(const char* str) {
        uint16_t len = (uint16_t)strlen(str);
        Write(len);
        Write(str, len);
    }
};

struct PacketReader {
    const uint8_t* data;
    uint16_t size;
    uint16_t pos = 0;
    
    PacketReader(const Packet& pkt) : data(pkt.data), size(pkt.size) {}
    
    template<typename T>
    T Read() {
        T val = {};
        if (pos + sizeof(T) <= size) {
            memcpy(&val, data + pos, sizeof(T));
            pos += sizeof(T);
        }
        return val;
    }
    
    std::string ReadString() {
        uint16_t len = Read<uint16_t>();
        std::string str;
        if (pos + len <= size) {
            str.assign((const char*)(data + pos), len);
            pos += len;
        }
        return str;
    }
};

namespace NetSocket {
    inline SOCKET Create() {
        return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    
    inline bool Bind(SOCKET sock, uint16_t port) {
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        return bind(sock, (sockaddr*)&addr, sizeof(addr)) == 0;
    }
    
    inline void SetNonBlocking(SOCKET sock) {
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
    }
    
    inline int Send(SOCKET sock, const void* data, int size, const sockaddr_in& to) {
        return sendto(sock, (const char*)data, size, 0, (const sockaddr*)&to, sizeof(to));
    }
    
    inline int Recv(SOCKET sock, void* data, int size, sockaddr_in& from) {
        int fromLen = sizeof(from);
        return recvfrom(sock, (char*)data, size, 0, (sockaddr*)&from, &fromLen);
    }
    
    inline void Close(SOCKET sock) {
        closesocket(sock);
    }
    
    inline sockaddr_in MakeAddr(const char* ip, uint16_t port) {
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        return addr;
    }
}

namespace NetClient {
    inline SOCKET g_socket = INVALID_SOCKET;
    inline sockaddr_in g_serverAddr = {};
    inline bool g_connected = false;
    inline uint32_t g_sequence = 0;
    inline uint32_t g_lastAck = 0;
    inline float g_ping = 0;
    inline std::vector<Packet> g_incoming;
    inline std::chrono::steady_clock::time_point g_lastPing;
    
    inline bool Connect(const char* ip, uint16_t port) {
        g_socket = NetSocket::Create();
        if (g_socket == INVALID_SOCKET) return false;
        NetSocket::SetNonBlocking(g_socket);
        g_serverAddr = NetSocket::MakeAddr(ip, port);
        
        Packet pkt;
        pkt.type = PacketType::Connect;
        pkt.sequence = g_sequence++;
        NetSocket::Send(g_socket, &pkt, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), g_serverAddr);
        
        g_connected = true;
        g_lastPing = std::chrono::steady_clock::now();
        Debug::Log("Connecting to %s:%d", ip, port);
        return true;
    }
    
    inline void Disconnect() {
        if (!g_connected) return;
        Packet pkt;
        pkt.type = PacketType::Disconnect;
        pkt.sequence = g_sequence++;
        NetSocket::Send(g_socket, &pkt, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), g_serverAddr);
        NetSocket::Close(g_socket);
        g_connected = false;
        Debug::Log("Disconnected");
    }
    
    inline void Update() {
        if (!g_connected) return;
        
        uint8_t buffer[1500];
        sockaddr_in from;
        int received;
        
        while ((received = NetSocket::Recv(g_socket, buffer, sizeof(buffer), from)) > 0) {
            if (received >= (int)(sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t))) {
                Packet pkt;
                memcpy(&pkt, buffer, received);
                
                if (pkt.type == PacketType::Pong) {
                    auto now = std::chrono::steady_clock::now();
                    g_ping = std::chrono::duration<float, std::milli>(now - g_lastPing).count();
                } else {
                    g_incoming.push_back(pkt);
                }
                
                if (pkt.sequence > g_lastAck) g_lastAck = pkt.sequence;
            }
        }
        
        static float pingTimer = 0;
        pingTimer += Internal::g_deltaTime;
        if (pingTimer >= 1.0f) {
            pingTimer = 0;
            Packet pkt;
            pkt.type = PacketType::Ping;
            pkt.sequence = g_sequence++;
            NetSocket::Send(g_socket, &pkt, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), g_serverAddr);
            g_lastPing = std::chrono::steady_clock::now();
        }
    }
    
    inline bool HasPacket() {
        return !g_incoming.empty();
    }
    
    inline Packet Receive() {
        if (g_incoming.empty()) return Packet();
        Packet pkt = g_incoming.front();
        g_incoming.erase(g_incoming.begin());
        return pkt;
    }
    
    inline void Send(const Packet& pkt) {
        Packet p = pkt;
        p.sequence = g_sequence++;
        p.ack = g_lastAck;
        NetSocket::Send(g_socket, &p, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t) + p.size, g_serverAddr);
    }
    
    inline bool IsConnected() { return g_connected; }
    inline float GetPing() { return g_ping; }
}

namespace NetServer {
    struct Client {
        ClientID id;
        sockaddr_in addr;
        uint32_t lastSeq = 0;
        uint32_t lastAck = 0;
        float lastPacketTime = 0;
        bool connected = false;
    };
    
    inline SOCKET g_socket = INVALID_SOCKET;
    inline uint16_t g_port = 0;
    inline int g_maxPlayers = 16;
    inline std::vector<Client> g_clients;
    inline std::vector<Packet> g_incoming;
    inline uint32_t g_sequence = 0;
    inline uint32_t g_nextClientID = 1;
    inline bool g_running = false;
    
    inline bool Start(uint16_t port, int maxPlayers) {
        g_socket = NetSocket::Create();
        if (g_socket == INVALID_SOCKET) return false;
        
        if (!NetSocket::Bind(g_socket, port)) {
            NetSocket::Close(g_socket);
            return false;
        }
        
        NetSocket::SetNonBlocking(g_socket);
        g_port = port;
        g_maxPlayers = maxPlayers;
        g_running = true;
        Debug::Log("Server started on port %d", port);
        return true;
    }
    
    inline void Stop() {
        if (!g_running) return;
        
        Packet pkt;
        pkt.type = PacketType::Disconnect;
        pkt.sequence = g_sequence++;
        for (auto& c : g_clients) {
            if (c.connected) {
                NetSocket::Send(g_socket, &pkt, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), c.addr);
            }
        }
        
        NetSocket::Close(g_socket);
        g_clients.clear();
        g_running = false;
        Debug::Log("Server stopped");
    }
    
    inline Client* FindClient(const sockaddr_in& addr) {
        for (auto& c : g_clients) {
            if (c.addr.sin_addr.s_addr == addr.sin_addr.s_addr && c.addr.sin_port == addr.sin_port) {
                return &c;
            }
        }
        return nullptr;
    }
    
    inline void Update() {
        if (!g_running) return;
        
        uint8_t buffer[1500];
        sockaddr_in from;
        int received;
        
        while ((received = NetSocket::Recv(g_socket, buffer, sizeof(buffer), from)) > 0) {
            if (received >= (int)(sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t))) {
                Packet pkt;
                memcpy(&pkt, buffer, received);
                
                Client* client = FindClient(from);
                
                if (pkt.type == PacketType::Connect) {
                    if (!client && (int)g_clients.size() < g_maxPlayers) {
                        Client newClient;
                        newClient.id = g_nextClientID++;
                        newClient.addr = from;
                        newClient.connected = true;
                        newClient.lastPacketTime = Internal::g_totalTime;
                        g_clients.push_back(newClient);
                        
                        Packet resp;
                        resp.type = PacketType::Connect;
                        resp.sender = newClient.id;
                        resp.sequence = g_sequence++;
                        NetSocket::Send(g_socket, &resp, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), from);
                        
                        Debug::Log("Client %d connected", newClient.id);
                    }
                } else if (pkt.type == PacketType::Disconnect) {
                    if (client) {
                        Debug::Log("Client %d disconnected", client->id);
                        client->connected = false;
                    }
                } else if (pkt.type == PacketType::Ping) {
                    if (client) {
                        client->lastPacketTime = Internal::g_totalTime;
                        Packet resp;
                        resp.type = PacketType::Pong;
                        resp.sequence = g_sequence++;
                        NetSocket::Send(g_socket, &resp, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), from);
                    }
                } else {
                    if (client) {
                        client->lastPacketTime = Internal::g_totalTime;
                        pkt.sender = client->id;
                        g_incoming.push_back(pkt);
                    }
                }
            }
        }
        
        for (auto it = g_clients.begin(); it != g_clients.end();) {
            if (!it->connected || Internal::g_totalTime - it->lastPacketTime > 30.0f) {
                Debug::Log("Client %d timed out", it->id);
                it = g_clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    inline bool HasPacket() {
        return !g_incoming.empty();
    }
    
    inline Packet Receive() {
        if (g_incoming.empty()) return Packet();
        Packet pkt = g_incoming.front();
        g_incoming.erase(g_incoming.begin());
        return pkt;
    }
    
    inline void SendTo(ClientID id, const Packet& pkt) {
        for (auto& c : g_clients) {
            if (c.id == id && c.connected) {
                Packet p = pkt;
                p.sequence = g_sequence++;
                NetSocket::Send(g_socket, &p, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t) + p.size, c.addr);
                break;
            }
        }
    }
    
    inline void Broadcast(const Packet& pkt) {
        for (auto& c : g_clients) {
            if (c.connected) {
                Packet p = pkt;
                p.sequence = g_sequence++;
                NetSocket::Send(g_socket, &p, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t) + p.size, c.addr);
            }
        }
    }
    
    inline void Kick(ClientID id) {
        for (auto& c : g_clients) {
            if (c.id == id) {
                Packet pkt;
                pkt.type = PacketType::Disconnect;
                pkt.sequence = g_sequence++;
                NetSocket::Send(g_socket, &pkt, sizeof(PacketType) + sizeof(ClientID) + sizeof(uint32_t) * 2 + sizeof(uint16_t), c.addr);
                c.connected = false;
                break;
            }
        }
    }
    
    inline int GetPlayerCount() {
        int count = 0;
        for (auto& c : g_clients) if (c.connected) count++;
        return count;
    }
    
    inline bool IsRunning() { return g_running; }
}

namespace Shield {
    inline void GenerateClientKey(uint8_t key[16]) {
        std::random_device rd;
        for (int i = 0; i < 16; i++) key[i] = (uint8_t)(rd() & 0xFF);
    }
    
    inline void CreateKeyPacket(uint8_t packet[64], const uint8_t key[16]) {
        memset(packet, 0, 64);
        for (int i = 0; i < 4; i++) {
            memcpy(packet + 8 + i * 12, key + i * 4, 4);
        }
        uint32_t checksum = 0;
        for (int i = 0; i < 56; i++) checksum += packet[i] * (i + 1);
        memcpy(packet + 56, &checksum, 4);
    }
    
    inline bool SetServerSalt(const uint8_t salt[16]) {
        return true;
    }
    
    inline bool Authenticate() {
        return true;
    }
    
    inline bool IsAuthenticated() {
        return true;
    }
    
    inline void DecryptCode() {
    }
}

}

#endif

