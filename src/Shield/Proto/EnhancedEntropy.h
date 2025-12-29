// FIX #4: Enhanced entropy pool (multiple sources)

#ifndef TITAN_ENHANCED_ENTROPY_H
#define TITAN_ENHANCED_ENTROPY_H

#include <cstdint>
#include <vector>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

namespace TitanShield {

class EnhancedEntropy {
public:
static std::vector<uint8_t> generate_secure_key(size_t bits) {
size_t bytes = bits / 8;
std::vector<uint8_t> key(bytes);

```
    uint64_t entropy_pool[16] = {0};
    size_t pool_index = 0;
    
    // Source 1: std::random_device
    std::random_device rd;
    for (int i = 0; i < 4; i++) {
        entropy_pool[pool_index++] = 
            (static_cast<uint64_t>(rd()) << 32) | rd();
    }
    
    // Source 2: High-res clock
    auto now = std::chrono::high_resolution_clock::now();
    entropy_pool[pool_index++] = now.time_since_epoch().count();
    
    // Source 3: System clock (different granularity)
    auto sys_now = std::chrono::system_clock::now();
    entropy_pool[pool_index++] = sys_now.time_since_epoch().count();
```

#ifdef _WIN32
// Source 4: Performance counter
LARGE_INTEGER perf;
QueryPerformanceCounter(&perf);
entropy_pool[pool_index++] = perf.QuadPart;

```
    // Source 5: Tick count
    entropy_pool[pool_index++] = GetTickCount64();
    
    // Source 6: Mouse cursor position
    POINT cursor;
    GetCursorPos(&cursor);
    entropy_pool[pool_index++] = 
        (static_cast<uint64_t>(cursor.x) << 32) | 
        static_cast<uint64_t>(cursor.y);
    
    // Source 7: Process/Thread IDs
    entropy_pool[pool_index++] = 
        (static_cast<uint64_t>(GetCurrentProcessId()) << 32) |
        static_cast<uint64_t>(GetCurrentThreadId());
    
    // Source 8: Memory address entropy
    entropy_pool[pool_index++] = reinterpret_cast<uint64_t>(&entropy_pool);
    entropy_pool[pool_index++] = reinterpret_cast<uint64_t>(&key);
```

#endif

```
    // Source 9: Hardware fingerprint
    entropy_pool[pool_index++] = HardwareInfo::GetCombinedFingerprint();
    
    // Source 10: Uninitialized stack memory (carefully controlled)
    volatile uint64_t stack_noise[4];
    for (int i = 0; i < 4 && pool_index < 16; i++) {
        entropy_pool[pool_index++] = stack_noise[i];
    }
    
    // Mix entropy pool using SHA-256
    std::vector<uint8_t> pool_bytes(sizeof(entropy_pool));
    std::memcpy(pool_bytes.data(), entropy_pool, sizeof(entropy_pool));
    
    auto mixed = RealSHA256::Hash(pool_bytes);
    
    // Generate key bytes by repeatedly hashing
    for (size_t i = 0; i < bytes; i += 32) {
        auto hash = RealSHA256::Hash(mixed);
        size_t copy_size = std::min(size_t(32), bytes - i);
        std::memcpy(key.data() + i, hash.data(), copy_size);
        
        // Update mixed for next iteration
        mixed = hash;
    }
    
    // Wipe entropy pool
    AdvancedCryptoEngine::SecureWipe(entropy_pool, sizeof(entropy_pool));
    
    return key;
}
```

};

} // namespace TitanShield

#endif

// Replace in AdvancedCryptoEngine::GenerateSecureKey (proto.cpp):
// OLD: std::random_device rd; std::mt19937_64 gen(rd()); …
// NEW: return EnhancedEntropy::generate_secure_key(bits);
