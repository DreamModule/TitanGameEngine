#ifndef TITAN_EVENTS_HPP
#define TITAN_EVENTS_HPP

#include "Titan_Core.hpp"

namespace Titan {

using EventID = u32;

struct EventContext {
    u32 senderID;
};

using EventHandler = void(*)(const EventContext&);

enum class EventScope { State, Global };

struct EventBus {
    static std::unordered_map<EventID, std::vector<EventHandler>> globalListeners;
    static std::unordered_map<EventID, std::vector<EventHandler>> stateListeners;

    static void Init();
    static void Subscribe(EventID id, EventHandler h, EventScope s = EventScope::State);
    static void Emit(EventID id, const EventContext& ctx);
    static void ResetState();
    static void Shutdown();
};
}
#endif
