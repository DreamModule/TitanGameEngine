/**
 * Input Manager Implementation
 * Titan Engine
 */

#include "InputManager.hpp"
#include <array>

namespace Titan::Input {

// ============================================================================
// Private State
// ============================================================================

static std::array<bool, 256> GKeyDown{};
static std::array<bool, 256> GKeyPressed{};
static std::array<bool, 256> GKeyReleased{};
static int GMouseX = 0;
static int GMouseY = 0;
static int GMouseDeltaX = 0;
static int GMouseDeltaY = 0;
static bool GMouseButtons[3] = {false, false, false};
static bool GMouseButtonsPressed[3] = {false, false, false};

// ============================================================================
// Manager Implementation
// ============================================================================

void Manager::OnKeyDown(uint32_t VirtualKey)
{
    if (VirtualKey < 256)
    {
        if (!GKeyDown[VirtualKey])
        {
            GKeyPressed[VirtualKey] = true;
        }
        GKeyDown[VirtualKey] = true;
    }
}

void Manager::OnKeyUp(uint32_t VirtualKey)
{
    if (VirtualKey < 256)
    {
        GKeyDown[VirtualKey] = false;
        GKeyReleased[VirtualKey] = true;
    }
}

bool Manager::IsKeyDown(uint32_t VirtualKey)
{
    if (VirtualKey < 256)
    {
        return GKeyDown[VirtualKey];
    }
    return false;
}

bool Manager::IsKeyPressed(uint32_t VirtualKey)
{
    if (VirtualKey < 256)
    {
        return GKeyPressed[VirtualKey];
    }
    return false;
}

bool Manager::IsKeyReleased(uint32_t VirtualKey)
{
    if (VirtualKey < 256)
    {
        return GKeyReleased[VirtualKey];
    }
    return false;
}

void Manager::OnMouseMove(int X, int Y)
{
    GMouseDeltaX = X - GMouseX;
    GMouseDeltaY = Y - GMouseY;
    GMouseX = X;
    GMouseY = Y;
}

void Manager::OnMouseButton(int Button, bool Down)
{
    if (Button >= 0 && Button < 3)
    {
        if (Down && !GMouseButtons[Button])
        {
            GMouseButtonsPressed[Button] = true;
        }
        GMouseButtons[Button] = Down;
    }
}

int Manager::GetMouseX()
{
    return GMouseX;
}

int Manager::GetMouseY()
{
    return GMouseY;
}

int Manager::GetMouseDeltaX()
{
    return GMouseDeltaX;
}

int Manager::GetMouseDeltaY()
{
    return GMouseDeltaY;
}

bool Manager::IsMouseButtonDown(int Button)
{
    if (Button >= 0 && Button < 3)
    {
        return GMouseButtons[Button];
    }
    return false;
}

bool Manager::IsMouseButtonPressed(int Button)
{
    if (Button >= 0 && Button < 3)
    {
        return GMouseButtonsPressed[Button];
    }
    return false;
}

void Manager::EndFrame()
{
    GKeyPressed.fill(false);
    GKeyReleased.fill(false);
    GMouseDeltaX = 0;
    GMouseDeltaY = 0;
    for (int i = 0; i < 3; i++)
    {
        GMouseButtonsPressed[i] = false;
    }
}

void Manager::Reset()
{
    GKeyDown.fill(false);
    GKeyPressed.fill(false);
    GKeyReleased.fill(false);
    GMouseX = 0;
    GMouseY = 0;
    GMouseDeltaX = 0;
    GMouseDeltaY = 0;
    for (int i = 0; i < 3; i++)
    {
        GMouseButtons[i] = false;
        GMouseButtonsPressed[i] = false;
    }
}

} // namespace Titan::Input


