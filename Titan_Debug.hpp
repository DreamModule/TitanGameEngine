/**
 * Titan Debug System
 * 
 * Immediate Mode Debug UI
 * Follows Unreal Engine naming conventions
 */

#ifndef TITAN_DEBUG_HPP
#define TITAN_DEBUG_HPP

#include "Titan_Core.hpp"
#include <vector>
#include <cstdio>
#include <cstdarg>

// Forward declarations
namespace Titan::Assets { struct Font; }
namespace Titan::Graphics { struct TextureHandle; }

namespace Titan::Debug {

// ============================================================================
// Draw Command Structure
// ============================================================================

struct FDrawCommand
{
    uint32 Type;          // 0 = Rect, 1 = Text
    float X, Y;
    float Width, Height;
    Math::Vec4 Color;
    char Text[64];
};

// ============================================================================
// Debug Context
// ============================================================================

struct FDebugContext
{
    float CursorX = 10.0f;
    float CursorY = 10.0f;
    float StartX = 10.0f;
    float StartY = 10.0f;
    float Padding = 5.0f;
    bool bIsActive = true;
    Assets::Font* Font = nullptr;
    std::vector<FDrawCommand> Commands;
    Math::Vec2 MousePosition;
    bool bMouseClicked = false;
};

// ============================================================================
// Global Context Access (defined in .cpp)
// ============================================================================

FDebugContext& GetContext();

// ============================================================================
// Debug API
// ============================================================================

void Initialize();
void Shutdown();

void Begin();
void End();

bool Button(const char* Label);
void Text(const char* Format, ...);
void Rect(float X, float Y, float Width, float Height, const Math::Vec4& Color);

void SetActive(bool bActive);
bool IsActive();

// ============================================================================
// Inline Implementations
// ============================================================================

inline void Begin()
{
    auto& Ctx = GetContext();
    if (!Ctx.bIsActive)
    {
        return;
    }

    Ctx.Commands.clear();
    Ctx.CursorX = Ctx.StartX;
    Ctx.CursorY = Ctx.StartY;
}

inline void End()
{
    // Nothing to do - commands are processed by render system
}

inline bool Button(const char* Label)
{
    auto& Ctx = GetContext();
    if (!Ctx.bIsActive)
    {
        return false;
    }

    float Width = 150.0f;
    float Height = 30.0f;
    
    bool bHover = (Ctx.MousePosition.x >= Ctx.CursorX && 
                   Ctx.MousePosition.x <= Ctx.CursorX + Width &&
                   Ctx.MousePosition.y >= Ctx.CursorY && 
                   Ctx.MousePosition.y <= Ctx.CursorY + Height);
    bool bPressed = bHover && Ctx.bMouseClicked;

    Math::Vec4 Color;
    if (bPressed)
    {
        Color = {0.1f, 0.6f, 0.1f, 1.0f};
    }
    else if (bHover)
    {
        Color = {0.4f, 0.4f, 0.4f, 0.9f};
    }
    else
    {
        Color = {0.2f, 0.2f, 0.2f, 0.8f};
    }

    FDrawCommand RectCmd;
    RectCmd.Type = 0;
    RectCmd.X = Ctx.CursorX;
    RectCmd.Y = Ctx.CursorY;
    RectCmd.Width = Width;
    RectCmd.Height = Height;
    RectCmd.Color = Color;
    RectCmd.Text[0] = '\0';
    Ctx.Commands.push_back(RectCmd);

    FDrawCommand TextCmd;
    TextCmd.Type = 1;
    TextCmd.X = Ctx.CursorX + 10.0f;
    TextCmd.Y = Ctx.CursorY + 20.0f;
    TextCmd.Width = 0;
    TextCmd.Height = 0;
    TextCmd.Color = {1.0f, 1.0f, 1.0f, 1.0f};
    strncpy(TextCmd.Text, Label, 63);
    TextCmd.Text[63] = '\0';
    Ctx.Commands.push_back(TextCmd);

    Ctx.CursorY += Height + Ctx.Padding;
    return bPressed;
}

inline void Text(const char* Format, ...)
{
    auto& Ctx = GetContext();
    if (!Ctx.bIsActive)
    {
        return;
    }

    char Buffer[64];
    va_list Args;
    va_start(Args, Format);
    vsnprintf(Buffer, 63, Format, Args);
    va_end(Args);
    Buffer[63] = '\0';

    FDrawCommand Cmd;
    Cmd.Type = 1;
    Cmd.X = Ctx.CursorX;
    Cmd.Y = Ctx.CursorY + 16.0f;
    Cmd.Width = 0;
    Cmd.Height = 0;
    Cmd.Color = {1.0f, 1.0f, 1.0f, 1.0f};
    strncpy(Cmd.Text, Buffer, 63);
    Cmd.Text[63] = '\0';
    Ctx.Commands.push_back(Cmd);

    Ctx.CursorY += 20.0f + Ctx.Padding;
}

inline void Rect(float X, float Y, float Width, float Height, const Math::Vec4& Color)
{
    auto& Ctx = GetContext();
    if (!Ctx.bIsActive)
    {
        return;
    }

    FDrawCommand Cmd;
    Cmd.Type = 0;
    Cmd.X = X;
    Cmd.Y = Y;
    Cmd.Width = Width;
    Cmd.Height = Height;
    Cmd.Color = Color;
    Cmd.Text[0] = '\0';
    Ctx.Commands.push_back(Cmd);
}

inline void SetActive(bool bActive)
{
    GetContext().bIsActive = bActive;
}

inline bool IsActive()
{
    return GetContext().bIsActive;
}

} // namespace Titan::Debug

#endif // TITAN_DEBUG_HPP
