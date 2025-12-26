#pragma once
#include "Titan_Input.hpp"
#include "Titan_Events.hpp"

namespace Titan {

struct Engine {
    struct Context {
        bool isRunning;
        ECS::World world;
        SnapshotStorage snapshots;
        EventBus events;
        InputManager input;
        StateManager stateMgr;
    };

    static Context* Get();
    static void Init();
    static void Shutdown();
};
}
