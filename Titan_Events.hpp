/**
 * Titan Events Header
 * 
 * Event bus system
 */

#ifndef TITAN_EVENTS_HPP
#define TITAN_EVENTS_HPP

#include "Titan_Core.hpp"
#include <unordered_map>
#include <vector>
#include <functional>

namespace Titan {

// ============================================================================
// Event Types
// ============================================================================

using EventID = uint32;

struct EventContext
{
    uint32 SenderID = 0;
    union
    {
        uint64 DataU64;
        void* Ptr;
    };
};

using EventHandler = std::function<void(const EventContext&)>;

// ============================================================================
// Event Bus
// ============================================================================

struct EventBus
{
    void Init()
    {
        Listeners.clear();
    }

    void Shutdown()
    {
        Listeners.clear();
    }

    /**
     * Subscribe to an event
     * @param ID - Event identifier
     * @param Handler - Callback function
     */
    void Subscribe(EventID ID, EventHandler Handler)
    {
        Listeners[ID].push_back(Handler);
    }

    /**
     * Emit an event to all subscribers
     * @param ID - Event identifier
     * @param Context - Event data
     */
    void Emit(EventID ID, const EventContext& Context)
    {
        auto It = Listeners.find(ID);
        if (It != Listeners.end())
        {
            for (auto& Handler : It->second)
            {
                Handler(Context);
            }
        }
    }

    /**
     * Clear all listeners for an event
     */
    void Clear(EventID ID)
    {
        Listeners.erase(ID);
    }

    /**
     * Clear all listeners
     */
    void ClearAll()
    {
        Listeners.clear();
    }

private:
    std::unordered_map<EventID, std::vector<EventHandler>> Listeners;
};

// ============================================================================
// Common Event IDs
// ============================================================================

namespace Events {

constexpr EventID OnEngineStart = 1;
constexpr EventID OnEngineShutdown = 2;
constexpr EventID OnSceneLoad = 10;
constexpr EventID OnSceneUnload = 11;
constexpr EventID OnEntityCreated = 20;
constexpr EventID OnEntityDestroyed = 21;
constexpr EventID OnCollision = 30;
constexpr EventID OnTriggerEnter = 31;
constexpr EventID OnTriggerExit = 32;

} // namespace Events

} // namespace Titan

#endif // TITAN_EVENTS_HPP
