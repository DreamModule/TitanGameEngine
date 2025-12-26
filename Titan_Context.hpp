#pragma once
#include "Titan_Core.hpp"

namespace Titan {
namespace ECS { struct World; }
struct SnapshotStorage;
struct EventBus;
struct InputManager;

struct FrameContext {
    ECS::World& world;
    SnapshotStorage& snapshots;
    EventBus& events;
    const InputManager& input;
    f32 dt;
};
}
