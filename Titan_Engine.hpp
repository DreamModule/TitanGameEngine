/**
 * Titan Engine Header
 * 
 * Main engine context and lifecycle management
 */

#ifndef TITAN_ENGINE_HPP
#define TITAN_ENGINE_HPP

#include "Titan_Core.hpp"
#include "Titan_ECS.hpp"
#include "Titan_Input.hpp"
#include "Titan_Events.hpp"
#include "Titan_Scheduler.hpp"
#include "Titan_Audio.hpp"
#include "Titan_Platform.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_State.hpp"

namespace Titan {

/**
 * Main Engine class - singleton access to engine systems
 */
struct Engine
{
    /**
     * Engine context containing all major systems
     */
    struct Context
    {
        bool isRunning = false;
        ECS::FWorld world;
        ECS::FScheduler scheduler;
        FSnapshotStorage snapshots;
        EventBus events;
        InputManager input;
        Audio::AudioManager audio;
        StateManager stateMgr;
    };

    /**
     * Get the global engine context
     */
    static Context* Get();

    /**
     * Initialize the engine
     * @param Title - Window title
     * @param Width - Window width in pixels
     * @param Height - Window height in pixels
     */
    static void Init(const char* Title, uint32 Width, uint32 Height);

    /**
     * Shutdown the engine and cleanup resources
     */
    static void Shutdown();

    /**
     * Check if engine is running
     */
    static bool IsRunning()
    {
        return Get()->isRunning;
    }

    /**
     * Request engine shutdown
     */
    static void Quit()
    {
        Get()->isRunning = false;
    }
};

} // namespace Titan

#endif // TITAN_ENGINE_HPP
