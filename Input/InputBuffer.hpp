#pragma once
#include “../ECS/Components/Transform.hpp”
#include <cstdint>
#include <deque>
#include <vector>

namespace Titan::Input {

using namespace ECS::Components;

enum class InputButton : uint32_t {
Forward = 1 << 0,
Backward = 1 << 1,
Left = 1 << 2,
Right = 1 << 3,
Jump = 1 << 4,
Crouch = 1 << 5,
Sprint = 1 << 6,
Fire = 1 << 7,
AltFire = 1 << 8,
Reload = 1 << 9,
Use = 1 << 10,
Melee = 1 << 11,
Grenade = 1 << 12,
Slot1 = 1 << 13,
Slot2 = 1 << 14,
Slot3 = 1 << 15,
Slot4 = 1 << 16,
NextWeapon = 1 << 17,
PrevWeapon = 1 << 18,
Walk = 1 << 19,
Zoom = 1 << 20
};

struct InputCommand {
uint32_t sequenceNumber;
uint32_t tick;
uint32_t timestamp;

```
Vec3 moveDirection;
Vec2 lookDelta;
Vec3 viewAngles;

uint32_t buttonMask;
uint32_t buttonPressed;
uint32_t buttonReleased;

float movementSpeed;

bool HasButton(InputButton btn) const {
    return (buttonMask & static_cast<uint32_t>(btn)) != 0;
}

bool WasPressed(InputButton btn) const {
    return (buttonPressed & static_cast<uint32_t>(btn)) != 0;
}

bool WasReleased(InputButton btn) const {
    return (buttonReleased & static_cast<uint32_t>(btn)) != 0;
}

void SetButton(InputButton btn, bool pressed) {
    uint32_t flag = static_cast<uint32_t>(btn);
    if (pressed) {
        if (!(buttonMask & flag)) {
            buttonPressed |= flag;
        }
        buttonMask |= flag;
    } else {
        if (buttonMask & flag) {
            buttonReleased |= flag;
        }
        buttonMask &= ~flag;
    }
}

InputCommand() 
    : sequenceNumber(0)
    , tick(0)
    , timestamp(0)
    , moveDirection(Vec3::Zero())
    , lookDelta(0, 0)
    , viewAngles(Vec3::Zero())
    , buttonMask(0)
    , buttonPressed(0)
    , buttonReleased(0)
    , movementSpeed(1.0f) {
}
```

};

class InputBuffer {
public:
InputBuffer();
~InputBuffer() = default;

```
void Init(size_t maxHistory = 256);
void Clear();

void RecordInput(const InputCommand& cmd);

bool GetInput(uint32_t sequenceNumber, InputCommand& outCmd) const;
bool GetInputByTick(uint32_t tick, InputCommand& outCmd) const;

const InputCommand* GetLatestInput() const;
const InputCommand* GetOldestInput() const;

void RemoveOldInputs(uint32_t beforeSequence);
void RemoveOldInputsByTick(uint32_t beforeTick);

size_t GetHistorySize() const { return history.size(); }
size_t GetMaxHistory() const { return maxHistory; }

bool IsEmpty() const { return history.empty(); }

uint32_t GetLatestSequenceNumber() const;
uint32_t GetOldestSequenceNumber() const;

void GetInputRange(uint32_t startSeq, uint32_t endSeq, std::vector<InputCommand>& outInputs) const;
```

private:
std::deque<InputCommand> history;
size_t maxHistory;
};

}
