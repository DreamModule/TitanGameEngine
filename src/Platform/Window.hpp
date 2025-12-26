#pragma once
#include <windows.h>

namespace Titan::Platform::Window {

bool Create(int width, int height, const char* title);
void PollEvents();
bool ShouldClose();
void SwapBuffers();
void Destroy();

HWND GetHWND();
HDC  GetHDC();

}
