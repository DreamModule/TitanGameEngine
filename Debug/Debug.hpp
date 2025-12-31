#pragma once
#include <string>

namespace Titan::Debug {

void Init();
void Shutdown();
void Begin();
void End();
void Text(int x, int y, const char* text);
void Rect(int x, int y, int w, int h, float r, float g, float b, float a);
bool Button(int x, int y, int w, int h, const char* text);

}
