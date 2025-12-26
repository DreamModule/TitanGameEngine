#pragma once
#include "Titan_Core.hpp"
#include "Titan_Input.hpp"
#include "Titan_Events.hpp"
#include "Titan_Scheduler.hpp"
#include "Titan_Audio.hpp"
#include "Titan_Platform.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_State.hpp"

namespace Titan {
    struct Engine {
        struct Context {
            bool isRunning;
            ECS::World world;
            ECS::Scheduler scheduler;
            SnapshotStorage snapshots;
            EventBus events;
            Input::Manager input;
            Audio::AudioManager audio;
            StateManager stateMgr;
        };
        static Context* Get();
        static void Init(const char* t, u32 w, u32 h);
        static void Shutdown();
        static bool IsRunning() { return Get()->isRunning; }
        static void Quit() { Get()->isRunning = false; }
    };
}
