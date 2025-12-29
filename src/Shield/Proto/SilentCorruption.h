// FIX #2: Silent Corruption instead of instant TerminateProcess

#ifndef TITAN_SILENT_CORRUPTION_H
#define TITAN_SILENT_CORRUPTION_H

#include <atomic>
#include <random>
#include <chrono>

namespace TitanShield {

class SilentCorruption {
private:
static std::atomic<uint32_t> corruption_level_;
static std::atomic<uint64_t> infection_time_;
static std::atomic<bool> is_infected_;

```
// Subtle gameplay corruption
struct CorruptionEffects {
    float damage_multiplier = 1.0f;
    float accuracy_penalty = 0.0f;
    bool corrupt_saves = false;
    bool induce_lag = false;
    uint32_t frames_until_crash = UINT32_MAX;
};

static CorruptionEffects current_effects_;
static std::mutex effects_mutex_;
```

public:
static void infect() {
if (!is_infected_.load()) {
is_infected_.store(true);
infection_time_.store(
std::chrono::steady_clock::now().time_since_epoch().count()
);
corruption_level_.store(1);
}
}

```
static bool is_infected() {
    return is_infected_.load();
}

// Call from game loop every frame
static void update_corruption() {
    if (!is_infected_.load()) return;
    
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    uint64_t infection_start = infection_time_.load();
    uint64_t elapsed_ns = now - infection_start;
    uint64_t elapsed_seconds = elapsed_ns / 1000000000ULL;
    
    std::lock_guard<std::mutex> lock(effects_mutex_);
    
    // Progressive corruption over 15-20 minutes
    if (elapsed_seconds < 60) {
        // First minute: nothing (let them think they got away)
        current_effects_.damage_multiplier = 1.0f;
    } else if (elapsed_seconds < 300) {
        // 1-5 minutes: subtle damage reduction
        current_effects_.damage_multiplier = 0.95f - (elapsed_seconds - 60) * 0.001f;
    } else if (elapsed_seconds < 600) {
        // 5-10 minutes: accuracy penalty
        current_effects_.accuracy_penalty = (elapsed_seconds - 300) * 0.0005f;
    } else if (elapsed_seconds < 900) {
        // 10-15 minutes: corrupt saves, induce lag
        current_effects_.corrupt_saves = true;
        current_effects_.induce_lag = (elapsed_seconds % 10 == 0);
    } else {
        // 15+ minutes: prepare for crash
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(300, 600);
        
        if (current_effects_.frames_until_crash == UINT32_MAX) {
            current_effects_.frames_until_crash = dis(gen);
        } else {
            current_effects_.frames_until_crash--;
        }
    }
    
    corruption_level_.fetch_add(1);
}

// Game should call these to get corrupted values
static float apply_damage_multiplier(float base_damage) {
    if (!is_infected_.load()) return base_damage;
    std::lock_guard<std::mutex> lock(effects_mutex_);
    return base_damage * current_effects_.damage_multiplier;
}

static bool should_corrupt_save() {
    if (!is_infected_.load()) return false;
    std::lock_guard<std::mutex> lock(effects_mutex_);
    return current_effects_.corrupt_saves;
}

static bool should_induce_lag() {
    if (!is_infected_.load()) return false;
    std::lock_guard<std::mutex> lock(effects_mutex_);
    return current_effects_.induce_lag;
}

static bool should_crash_now() {
    if (!is_infected_.load()) return false;
    std::lock_guard<std::mutex> lock(effects_mutex_);
    
    if (current_effects_.frames_until_crash == 0) {
        // Crash in random location via nullptr access
        volatile int* ptr = nullptr;
        *ptr = 42;  // This will crash
        return true;
    }
    
    return false;
}
```

};

std::atomic<uint32_t> SilentCorruption::corruption_level_{0};
std::atomic<uint64_t> SilentCorruption::infection_time_{0};
std::atomic<bool> SilentCorruption::is_infected_{false};
SilentCorruption::CorruptionEffects SilentCorruption::current_effects_{};
std::mutex SilentCorruption::effects_mutex_;

} // namespace TitanShield

#endif

// Replace in AntiReverseEngineering (proto.cpp):
// OLD: void TriggerAntiTamperHard() { TerminateProcess(…); }
// NEW: void TriggerAntiTamperHard() { SilentCorruption::infect(); }
//
// In game loop:
// SilentCorruption::update_corruption();
// SilentCorruption::should_crash_now();
//
// In damage calculation:
// float final_damage = SilentCorruption::apply_damage_multiplier(base_damage);
