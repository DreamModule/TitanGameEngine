#include “InputBuffer.hpp”
#include <algorithm>

namespace Titan::Input {

InputBuffer::InputBuffer()
: maxHistory(256) {
}

void InputBuffer::Init(size_t max) {
maxHistory = max;
history.clear();
}

void InputBuffer::Clear() {
history.clear();
}

void InputBuffer::RecordInput(const InputCommand& cmd) {
if (history.size() >= maxHistory) {
history.pop_front();
}

```
history.push_back(cmd);
```

}

bool InputBuffer::GetInput(uint32_t sequenceNumber, InputCommand& outCmd) const {
for (const auto& input : history) {
if (input.sequenceNumber == sequenceNumber) {
outCmd = input;
return true;
}
}
return false;
}

bool InputBuffer::GetInputByTick(uint32_t tick, InputCommand& outCmd) const {
for (const auto& input : history) {
if (input.tick == tick) {
outCmd = input;
return true;
}
}
return false;
}

const InputCommand* InputBuffer::GetLatestInput() const {
return history.empty() ? nullptr : &history.back();
}

const InputCommand* InputBuffer::GetOldestInput() const {
return history.empty() ? nullptr : &history.front();
}

void InputBuffer::RemoveOldInputs(uint32_t beforeSequence) {
while (!history.empty() && history.front().sequenceNumber < beforeSequence) {
history.pop_front();
}
}

void InputBuffer::RemoveOldInputsByTick(uint32_t beforeTick) {
while (!history.empty() && history.front().tick < beforeTick) {
history.pop_front();
}
}

uint32_t InputBuffer::GetLatestSequenceNumber() const {
return history.empty() ? 0 : history.back().sequenceNumber;
}

uint32_t InputBuffer::GetOldestSequenceNumber() const {
return history.empty() ? 0 : history.front().sequenceNumber;
}

void InputBuffer::GetInputRange(uint32_t startSeq, uint32_t endSeq, std::vector<InputCommand>& outInputs) const {
outInputs.clear();

```
for (const auto& input : history) {
    if (input.sequenceNumber >= startSeq && input.sequenceNumber <= endSeq) {
        outInputs.push_back(input);
    }
}
```

}

}
