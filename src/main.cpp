#include "Core/Engine.hpp"

int main() {
    Titan::Core::EngineContext ctx;

    if (!Titan::Core::Engine::Init(ctx))
        return -1;

    while (ctx.running) {
        Titan::Core::Engine::Update(ctx);
    }

    Titan::Core::Engine::Shutdown(ctx);
    return 0;
}
