// FIX #1: Context-Aware Key Storage (no plain keys in memory)

#ifndef TITAN_CONTEXT_AWARE_KEY_H
#define TITAN_CONTEXT_AWARE_KEY_H

#include <cstdint>
#include <vector>
#include <array>

namespace TitanShield {

// Volatile context that changes frequently
class VolatileContext {
private:
static std::atomic<uint64_t> frame_counter_;
static std::atomic<uint64_t> operation_counter_;
static std::atomic<uint64_t> last_mouse_pos_;

public:
static void increment_frame() {
frame_counter_.fetch_add(1);
}

```
static void update_mouse(uint64_t pos) {
    last_mouse_pos_.store(pos);
}

static uint64_t get_xor_mask() {
    return frame_counter_.load() ^ 
           operation_counter_.load() ^ 
           last_mouse_pos_.load() ^
           reinterpret_cast<uint64_t>(&frame_counter_);
}
```

};

std::atomic<uint64_t> VolatileContext::frame_counter_{0};
std::atomic<uint64_t> VolatileContext::operation_counter_{0};
std::atomic<uint64_t> VolatileContext::last_mouse_pos_{0};

// Key that never exists in plain form in memory
class ContextAwareKey {
private:
std::vector<uint64_t> encrypted_chunks_;
uint64_t base_xor_;

public:
ContextAwareKey(const std::vector<uint8_t>& plain_key) {
base_xor_ = HardwareInfo::GetCombinedFingerprint();

```
    // Split key into 8-byte chunks and encrypt with context
    encrypted_chunks_.resize((plain_key.size() + 7) / 8);
    
    for (size_t i = 0; i < encrypted_chunks_.size(); i++) {
        uint64_t chunk = 0;
        for (size_t j = 0; j < 8 && (i * 8 + j) < plain_key.size(); j++) {
            chunk |= static_cast<uint64_t>(plain_key[i * 8 + j]) << (j * 8);
        }
        
        uint64_t mask = base_xor_ ^ VolatileContext::get_xor_mask() ^ (i * 0x517CC1B727220A95ULL);
        encrypted_chunks_[i] = chunk ^ mask;
    }
}

// Decrypt key into stack buffer (auto-wiped)
class TemporaryKey {
    uint8_t stack_buffer_[64];
    size_t size_;
    
public:
    TemporaryKey(const std::vector<uint64_t>& encrypted_chunks, uint64_t base_xor) {
        size_ = encrypted_chunks.size() * 8;
        
        for (size_t i = 0; i < encrypted_chunks.size(); i++) {
            uint64_t mask = base_xor ^ VolatileContext::get_xor_mask() ^ (i * 0x517CC1B727220A95ULL);
            uint64_t chunk = encrypted_chunks[i] ^ mask;
            
            for (size_t j = 0; j < 8 && (i * 8 + j) < size_; j++) {
                stack_buffer_[i * 8 + j] = static_cast<uint8_t>((chunk >> (j * 8)) & 0xFF);
            }
        }
    }
    
    ~TemporaryKey() {
        // Stack buffer auto-wiped
        AdvancedCryptoEngine::SecureWipe(stack_buffer_, sizeof(stack_buffer_));
    }
    
    const uint8_t* data() const { return stack_buffer_; }
    size_t size() const { return size_; }
};

TemporaryKey decrypt_temporary() const {
    return TemporaryKey(encrypted_chunks_, base_xor_);
}
```

};

} // namespace TitanShield

#endif

// Replace in TitanShieldCore class (proto.h):
// OLD: std::vector<uint8_t> masterKey_;
// NEW: ContextAwareKey masterKey_;
//
// Usage:
// auto temp_key = masterKey_.decrypt_temporary();
// Use temp_key.data() immediately
// Key is auto-wiped when temp_key goes out of scope
