#ifndef TITAN_WINDOW_HPP
#define TITAN_WINDOW_HPP
#include "Titan_Core.hpp"
namespace Titan {
struct Window {
    struct System {
        static void Init(u32 w, u32 h, const char* t);
        static bool Update();
        static void SwapBuffers();
        static void Shutdown();
    };
};
}
#endif
