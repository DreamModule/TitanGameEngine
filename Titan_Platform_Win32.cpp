#ifdef _WIN32
#include "Titan_Platform.hpp"
#include <windows.h>
static HWND g_Hwnd; static HDC g_HDC; static HGLRC g_HGLRC; static bool g_Run;
static Titan::Math::Vec2 g_MousePos; static LARGE_INTEGER g_Freq, g_Start; static bool g_Keys[512];
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) { 
    if(m==WM_CLOSE) g_Run=false; else if(m==WM_KEYDOWN) { if(w<512) g_Keys[w]=true; } else if(m==WM_KEYUP) { if(w<512) g_Keys[w]=false; }
    return DefWindowProc(h,m,w,l); 
}
namespace Titan::Platform {
    void Init(u32 w, u32 h, const char* t) {
        WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=GetModuleHandle(0); wc.lpszClassName="T"; RegisterClass(&wc);
        g_Hwnd=CreateWindow("T",t,WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,w,h,0,0,wc.hInstance,0);
        g_HDC=GetDC(g_Hwnd); PIXELFORMATDESCRIPTOR p={sizeof(p),1,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,32,0,0,0,0,0,0,0,0,0,0,0,0,0,24,8,0,0,0,0,0,0};
        SetPixelFormat(g_HDC,ChoosePixelFormat(g_HDC,&p),&p); g_HGLRC=wglCreateContext(g_HDC); wglMakeCurrent(g_HDC,g_HGLRC); 
        QueryPerformanceFrequency(&g_Freq); QueryPerformanceCounter(&g_Start); g_Run=true;
    }
    bool PumpMessages() { MSG m; while(PeekMessage(&m,0,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessage(&m);} POINT p; GetCursorPos(&p); ScreenToClient(g_Hwnd, &p); g_MousePos={(f32)p.x,(f32)p.y}; return g_Run; }
    void SwapBuffers() { ::SwapBuffers(g_HDC); } void Shutdown() { wglDeleteContext(g_HGLRC); DestroyWindow(g_Hwnd); }
    bool GetKeyDown(KeyCode k) { int v=0; if(k==KeyCode::W)v='W'; else if(k==KeyCode::Space)v=VK_SPACE; /*...*/ if(v<512) return g_Keys[v]; return false; }
    Math::Vec2 GetMousePos() { return g_MousePos; }
    f32 GetTime() { LARGE_INTEGER n; QueryPerformanceCounter(&n); return (f32)(n.QuadPart-g_Start.QuadPart)/(f32)g_Freq.QuadPart; }
}
#endif
