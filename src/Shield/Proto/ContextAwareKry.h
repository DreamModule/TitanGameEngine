// FIX #1 CORRECTED: Rolling Encryption (key re-encrypts on context change)

#ifndef TITAN_CONTEXT_AWARE_KEY_H
#define TITAN_CONTEXT_AWARE_KEY_H

#include <cstdint>
#include <vector>
#include <array>
#include <atomic>
#include <mutex>

namespace TitanShield {

class VolatileContext {
private:
static std::atomic<uint64_t> frame_counter_;
static std::atomic<uint64_t> last_xor_mask_;

public:
static void increment_frame() {
frame_counter_.fetch_add(1);
}

```
static uint64_t get_current_mask() {
    uint64_t frame = frame_counter_.load();
    
    // Mask changes every 16 frames (not every frame for stability)
    uint64_t stable_frame = (frame / 16) * 16;
    
    uint64_t mask = stable_frame ^ 
                   reinterpret_cast<uint64_t>(&frame_counter_) ^
                   0x517CC1B727220A95ULL;
    
    return mask;
}

static uint64_t get_previous_mask() {
    return last_xor_mask_.load();
}

static void update_mask(uint64_t new_mask) {
    last_xor_mask_.store(new_mask);
}
```

};

std::atomic<uint64_t> VolatileContext::frame_counter_{0};
std::atomic<uint64_t> VolatileContext::last_xor_mask_{0};

// Key that auto-rotates encryption based on context
class RollingEncryptedKey {
private:
std::vector<uint64_t> encrypted_chunks_;
uint64_t base_xor_;
uint64_t current_context_mask_;
mutable std::mutex rotation_mutex_;

```
void rotate_encryption() {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    
    uint64_t new_mask = VolatileContext::get_current_mask();
    
    // Only rotate if mask changed
    if (new_mask == current_context_mask_) return;
    
    uint64_t old_mask = current_context_mask_;
    
    // Decrypt with old mask, re-encrypt with new mask
    for (size_t i = 0; i < encrypted_chunks_.size(); i++) {
        uint64_t old_xor = base_xor_ ^ old_mask ^ (i * 0x9E3779B97F4A7C15ULL);
        uint64_t new_xor = base_xor_ ^ new_mask ^ (i * 0x9E3779B97F4A7C15ULL);
        
        // Decrypt then re-encrypt
        uint64_t plain = encrypted_chunks_[i] ^ old_xor;
        encrypted_chunks_[i] = plain ^ new_xor;
    }
    
    current_context_mask_ = new_mask;
    VolatileContext::update_mask(new_mask);
}
```

public:
RollingEncryptedKey(const std::vector<uint8_t>& plain_key) {
base_xor_ = HardwareInfo::GetCombinedFingerprint();
current_context_mask_ = VolatileContext::get_current_mask();

```
    encrypted_chunks_.resize((plain_key.size() + 7) / 8);
    
    for (size_t i = 0; i < encrypted_chunks_.size(); i++) {
        uint64_t chunk = 0;
        for (size_t j = 0; j < 8 && (i * 8 + j) < plain_key.size(); j++) {
            chunk |= static_cast<uint64_t>(plain_key[i * 8 + j]) << (j * 8);
        }
        
        uint64_t xor_mask = base_xor_ ^ current_context_mask_ ^ (i * 0x9E3779B97F4A7C15ULL);
        encrypted_chunks_[i] = chunk ^ xor_mask;
    }
    
    VolatileContext::update_mask(current_context_mask_);
}

class TemporaryKey {
    uint8_t stack_buffer_[64];
    size_t size_;
    
public:
    TemporaryKey(const std::vector<uint64_t>& encrypted_chunks, uint64_t base_xor, uint64_t context_mask) {
        size_ = encrypted_chunks.size() * 8;
        
        for (size_t i = 0; i < encrypted_chunks.size(); i++) {
            uint64_t xor_mask = base_xor ^ context_mask ^ (i * 0x9E3779B97F4A7C15ULL);
            uint64_t chunk = encrypted_chunks[i] ^ xor_mask;
            
            for (size_t j = 0; j < 8 && (i * 8 + j) < size_; j++) {
                stack_buffer_[i * 8 + j] = static_cast<uint8_t>((chunk >> (j * 8)) & 0xFF);
            }
        }
    }
    
    ~TemporaryKey() {
        AdvancedCryptoEngine::SecureWipe(stack_buffer_, sizeof(stack_buffer_));
    }
    
    const uint8_t* data() const { return stack_buffer_; }
    size_t size() const { return size_; }
};

TemporaryKey decrypt_temporary() {
    // Auto-rotate before decryption
    rotate_encryption();
    
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return TemporaryKey(encrypted_chunks_, base_xor_, current_context_mask_);
}

// Call periodically from game loop to rotate encryption
void maintain_rotation() {
    rotate_encryption();
}
```

};

} // namespace TitanShield

#endif

// Usage in TitanShieldCore (proto.h):
// Replace: std::vector<uint8_t> masterKey_;
// With:    RollingEncryptedKey masterKey_;

// In MonitoringLoop() add:
// masterKey_.maintain_rotation();  // Rotates key encryption every 16 frames

// When using key:
// auto temp_key = masterKey_.decrypt_temporary();
// // Use temp_key.data() immediately
// // Auto-wiped when temp_key goes out of scope
