#ifndef TITAN_ENGINE_HPP
#define TITAN_ENGINE_HPP
#include "Titan_ECS.hpp"
#include "Titan_State.hpp"
namespace Titan {
struct Engine {
    struct Context { bool run; ECS::World w; StateManager sm; };
    static Context* Get();
    static void Init(const char* t, u32 w, u32 h);
    static void Shutdown();
    static void Quit();
    static bool IsRunning();
};
}
#endif
