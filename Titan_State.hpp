/**
 * Titan State Header
 * 
 * Game state machine
 */

#ifndef TITAN_STATE_HPP
#define TITAN_STATE_HPP

#include "Titan_Core.hpp"

namespace Titan {

// Forward declaration
struct Engine;

// ============================================================================
// State Interface
// ============================================================================

struct IState
{
    virtual ~IState() = default;
    
    virtual void OnEnter() {}
    virtual void OnUpdate(float DeltaTime) {}
    virtual void OnExit() {}
    virtual const char* GetName() const = 0;
};

// ============================================================================
// State Manager
// ============================================================================

struct StateManager
{
    IState* CurrentState = nullptr;
    IState* NextState = nullptr;
    Engine::Context* EngineContext = nullptr;

    void Init(Engine::Context* Context)
    {
        EngineContext = Context;
    }

    void SwitchState(IState* NewState)
    {
        NextState = NewState;
    }

    void Update(float DeltaTime)
    {
        // Handle state transition
        if (NextState != nullptr)
        {
            if (CurrentState != nullptr)
            {
                CurrentState->OnExit();
            }
            
            CurrentState = NextState;
            NextState = nullptr;
            
            if (CurrentState != nullptr)
            {
                CurrentState->OnEnter();
            }
        }

        // Update current state
        if (CurrentState != nullptr)
        {
            CurrentState->OnUpdate(DeltaTime);
        }
    }

    void Shutdown()
    {
        if (CurrentState != nullptr)
        {
            CurrentState->OnExit();
            CurrentState = nullptr;
        }
        NextState = nullptr;
    }
};

} // namespace Titan

#endif // TITAN_STATE_HPP
