#include "Titan_Engine.hpp"

namespace Titan {
static Engine::Context gCtx;

Engine::Context* Engine::Get() { return &gCtx; }

void Engine::Init() {
    gCtx.isRunning = true;
    gCtx.events.Init();
    gCtx.input.Init();
    gCtx.stateMgr.Init(&gCtx);
}

void Engine::Shutdown() {
    gCtx.stateMgr.Shutdown();
    gCtx.events.Shutdown();
    gCtx.isRunning = false;
}
}
