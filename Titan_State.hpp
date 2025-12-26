#ifndef TITAN_STATE_HPP
#define TITAN_STATE_HPP

#include "Titan_Core.hpp"

namespace Titan {

struct IState {
    virtual ~IState() {}
    virtual void OnEnter() {}
    virtual void OnUpdate(f32 dt) {}
    virtual void OnExit() {}
    virtual const char* GetName() const =0;
};

struct StateManager {
    IState* curr = nullptr;
    IState* next = nullptr;

    void SwitchState(IState* s);
    void Update(f32 dt);
    void Shutdown();
};
}
#endif
