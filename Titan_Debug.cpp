/**
 * Titan Debug System Implementation
 */

#include "Titan_Debug.hpp"
#include "Titan_Assets.hpp"

namespace Titan::Debug {

// ============================================================================
// Global Context (Meyer's Singleton)
// ============================================================================

FDebugContext& GetContext()
{
    static FDebugContext Context;
    return Context;
}

// ============================================================================
// API Implementation
// ============================================================================

void Initialize()
{
    auto& Ctx = GetContext();
    Ctx.Font = Assets::Loader::LoadFont("assets/arial.ttf", 16.0f);
    Ctx.bIsActive = true;
    Ctx.CursorX = 10.0f;
    Ctx.CursorY = 10.0f;
    Ctx.StartX = 10.0f;
    Ctx.StartY = 10.0f;
    Ctx.Padding = 5.0f;
}

void Shutdown()
{
    auto& Ctx = GetContext();
    Ctx.Commands.clear();
    Ctx.Font = nullptr;
}

} // namespace Titan::Debug


