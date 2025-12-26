#pragma once
#include <unordered_map>
#include <vector>
#include "Titan_Core.hpp"

namespace Titan {

using EventID = u32;

struct EventContext {
    u32 senderID;
};

using EventHandler = void(*)(const EventContext&);

enum class EventScope { State, Global };

struct EventBus {
    std::unordered_map<EventID, std::vector<EventHandler>> globalListeners;
    std::unordered_map<EventID, std::vector<EventHandler>> stateListeners;

    void Init() {
        globalListeners.clear();
        stateListeners.clear();
    }

    void Subscribe(EventID id, EventHandler h, EventScope s = EventScope::State) {
        if (s == EventScope::Global) globalListeners[id].push_back(h);
        else stateListeners[id].push_back(h);
    }

    void Emit(EventID id, const EventContext& ctx) {
        if (globalListeners.count(id))
            for (auto& h : globalListeners[id]) h(ctx);
        if (stateListeners.count(id))
            for (auto& h : stateListeners[id]) h(ctx);
    }

    void ResetState() { stateListeners.clear(); }
    void Shutdown() { globalListeners.clear(); stateListeners.clear(); }
};
}
