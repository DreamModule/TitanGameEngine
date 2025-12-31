#pragma once
#include <windows.h>

namespace Titan::Graphics::Device {

bool Init(HWND hwnd, HDC hdc);
void BeginFrame();
void EndFrame();
void Shutdown();

}
