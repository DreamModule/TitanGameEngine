#ifndef TITAN_EVENTS_HPP
#define TITAN_EVENTS_HPP
#include "Titan_Core.hpp"
#include <unordered_map>
#include <vector>
namespace Titan {
    using EventID = u32;
    struct EventContext { u32 senderID; union { u64 dataU64; void* ptr; }; };
    using EventHandler = void(*)(const EventContext&);
    struct EventBus {
        std::unordered_map<EventID, std::vector<EventHandler>> l;
        void Subscribe(EventID id, EventHandler h) { l[id].push_back(h); }
        void Emit(EventID id, const EventContext& ctx) { if(l.count(id)) for(auto& h:l[id]) h(ctx); }
        void Init(){l.clear();} void Shutdown(){l.clear();}
    };
}
#endif
