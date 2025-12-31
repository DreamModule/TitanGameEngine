/**
 * Titan UI Header
 * 
 * User interface components and rendering
 */

#ifndef TITAN_UI_HPP
#define TITAN_UI_HPP

#include "Titan_ECS.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_Debug.hpp"
#include "Titan_Assets.hpp"

namespace Titan::UI {

// ============================================================================
// UI Components
// ============================================================================

struct FRectComponent
{
    Math::Vec2 Size{100, 100};
    Math::Vec4 Color{1, 1, 1, 1};
    Math::Vec4 DefaultColor{1, 1, 1, 1};
    Math::Vec4 HoverColor{0.8f, 0.8f, 0.8f, 1};
    Math::Vec4 PressedColor{0.6f, 0.6f, 0.6f, 1};
    int32 ZIndex = 0;
};

struct FTextComponent
{
    char Text[128] = "";
    Assets::Font* Font = nullptr;
    Math::Vec4 Color{1, 1, 1, 1};
    bool bCentered = false;
    int32 ZIndex = 0;
    Graphics::BufferHandle MeshBuffer;
    uint32 VertexCount = 0;
    bool bDirty = true;

    void SetText(const char* NewText)
    {
        strncpy(Text, NewText, 127);
        Text[127] = '\0';
        bDirty = true;
    }
};

struct FJoystickComponent
{
    const char* AxisHorizontal = nullptr;
    const char* AxisVertical = nullptr;
    float Radius = 64.0f;
    float KnobRadius = 24.0f;
    bool bIsDragging = false;
    uint32 ActivePointerID = 0;
    Math::Vec2 BasePosition;
    ECS::FEntityID KnobEntity = ECS::NULL_ENTITY;
};

// ============================================================================
// UI Systems
// ============================================================================

/**
 * Debug UI Render System
 * Renders immediate mode debug UI
 */
struct FDebugRenderSystem : ECS::ISystem
{
    Graphics::BufferHandle VBO;
    Graphics::ShaderHandle Shader;

    void Init(ECS::FWorld& World) override;
    void Update(ECS::FWorld& World, float DeltaTime) override;
    int GetPriority() const override { return 10000; }
};

/**
 * UI Render System
 * Renders UI rectangles and text
 */
struct FRenderSystem : ECS::ISystem
{
    void Update(ECS::FWorld& World, float DeltaTime) override;
    int GetPriority() const override { return 1000; }
};

/**
 * Virtual Joystick System
 * Handles touch/mouse joystick controls
 */
struct FJoystickSystem : ECS::ISystem
{
    void Update(ECS::FWorld& World, float DeltaTime) override;
    int GetPriority() const override { return 10; }
};

// ============================================================================
// Legacy Aliases
// ============================================================================

using RectComponent = FRectComponent;
using TextComponent = FTextComponent;
using JoystickComponent = FJoystickComponent;
using DebugRenderSystem = FDebugRenderSystem;
using RenderSystem = FRenderSystem;
using JoystickSystem = FJoystickSystem;

} // namespace Titan::UI

#endif // TITAN_UI_HPP
