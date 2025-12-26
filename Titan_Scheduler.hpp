#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include "Titan_System.hpp"

namespace Titan::ECS {

struct Scheduler {
    std::vector<ISystem*> initSystems;
    std::vector<ISystem*> simSystems;
    std::vector<ISystem*> presSystems;

    f32 fixedTimestep = 1.0f / 60.0f;
    f32 accumulator = 0.0f;

    ~Scheduler() {
        for (auto* s : initSystems) delete s;
        for (auto* s : simSystems) delete s;
        for (auto* s : presSystems) delete s;
    }

    void Register(ISystem* s, SystemGroup g) {
        if (g == SystemGroup::Initialization) initSystems.push_back(s);
        if (g == SystemGroup::Simulation) simSystems.push_back(s);
        if (g == SystemGroup::Presentation) presSystems.push_back(s);
    }

    void SortGroup(std::vector<ISystem*>& systems) {
        std::unordered_map<SystemID, ISystem*> map;
        std::unordered_map<SystemID, u32> state;
        std::vector<ISystem*> out;

        for (auto* s : systems) map[s->GetID()] = s;

        auto dfs = [&](auto&& self, ISystem* s) -> void {
            auto id = s->GetID();
            if (state[id] == 1) std::terminate();
            if (state[id] == 2) return;
            state[id] = 1;
            for (auto dep : s->GetDependencies())
                if (map.count(dep)) self(self, map[dep]);
            state[id] = 2;
            out.push_back(s);
        };

        for (auto* s : systems)
            if (!state[s->GetID()]) dfs(dfs, s);

        systems = out;
    }

    void Init(World& w) {
        SortGroup(initSystems);
        SortGroup(simSystems);
        SortGroup(presSystems);
        for (auto* s : initSystems) s->Init(w);
        for (auto* s : simSystems) s->Init(w);
        for (auto* s : presSystems) s->Init(w);
        for (auto* s : initSystems) {
            FrameContext ctx{w, *(SnapshotStorage*)nullptr, *(EventBus*)nullptr, *(InputManager*)nullptr, 0};
            s->Update(ctx);
        }
    }

    void Update(Engine::Context& ctx, f32 dt) {
        accumulator += dt;
        FrameContext fc{ctx.world, ctx.snapshots, ctx.events, ctx.input, 0};
        while (accumulator >= fixedTimestep) {
            fc.dt = fixedTimestep;
            for (auto* s : simSystems) s->Update(fc);
            ctx.world.Flush();
            accumulator -= fixedTimestep;
        }
        fc.dt = dt;
        for (auto* s : presSystems) s->Update(fc);
    }

    void Shutdown(World& w) {
        for (auto it = presSystems.rbegin(); it != presSystems.rend(); ++it) (*it)->Shutdown(w);
        for (auto it = simSystems.rbegin(); it != simSystems.rend(); ++it) (*it)->Shutdown(w);
        for (auto it = initSystems.rbegin(); it != initSystems.rend(); ++it) (*it)->Shutdown(w);
    }
};
}
