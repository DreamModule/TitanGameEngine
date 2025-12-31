#ifndef TITAN_NET_TYPES_HPP
#define TITAN_NET_TYPES_HPP

#include <cstdint>
#include <cstring>
#include <vector>
#include <deque>
#include <cmath>

namespace Titan {
namespace Net {

constexpr int TICKRATE = 128;
constexpr float TICK_INTERVAL = 1.0f / TICKRATE;
constexpr int SNAPSHOT_RATE = 128;
constexpr float SNAPSHOT_INTERVAL = 1.0f / SNAPSHOT_RATE;
constexpr int SNAPSHOT_BUFFER_SIZE = 128;
constexpr int INPUT_BUFFER_SIZE = 128;
constexpr float INTERP_DELAY_LAN = 0.025f;
constexpr float INTERP_DELAY_ONLINE = 0.050f;
constexpr int MAX_PLAYERS = 32;
constexpr int MAX_INPUTS_PER_PACKET = 8;

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float LengthSq() const { return x * x + y * y + z * z; }
    Vec3 Normalized() const { float l = Length(); return l > 0.0001f ? Vec3{x/l, y/l, z/l} : *this; }
    
    static Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
};

struct PlayerInput {
    uint32_t tick = 0;
    uint32_t sequence = 0;
    uint8_t buttons = 0;
    float moveX = 0;
    float moveY = 0;
    float yaw = 0;
    float pitch = 0;
    
    enum Buttons : uint8_t {
        Forward = 1 << 0,
        Back    = 1 << 1,
        Left    = 1 << 2,
        Right   = 1 << 3,
        Jump    = 1 << 4,
        Crouch  = 1 << 5,
        Sprint  = 1 << 6,
        Fire    = 1 << 7
    };
    
    bool HasButton(Buttons b) const { return (buttons & b) != 0; }
    void SetButton(Buttons b, bool v) { if (v) buttons |= b; else buttons &= ~b; }
};

struct PlayerState {
    uint32_t clientId = 0;
    uint32_t lastProcessedInput = 0;
    Vec3 position;
    Vec3 velocity;
    float yaw = 0;
    float pitch = 0;
    bool onGround = true;
    bool alive = true;
    uint8_t health = 100;
    
    void ApplyInput(const PlayerInput& input, float dt) {
        float speed = input.HasButton(PlayerInput::Sprint) ? 7.0f : 4.5f;
        if (input.HasButton(PlayerInput::Crouch)) speed *= 0.4f;
        
        float sinYaw = sinf(yaw);
        float cosYaw = cosf(yaw);
        
        Vec3 forward = {sinYaw, 0, -cosYaw};
        Vec3 right = {cosYaw, 0, sinYaw};
        
        Vec3 wishDir = {0, 0, 0};
        if (input.HasButton(PlayerInput::Forward)) wishDir = wishDir - forward;
        if (input.HasButton(PlayerInput::Back)) wishDir = wishDir + forward;
        if (input.HasButton(PlayerInput::Left)) wishDir = wishDir + right;
        if (input.HasButton(PlayerInput::Right)) wishDir = wishDir - right;
        
        if (wishDir.LengthSq() > 0.01f) {
            wishDir = wishDir.Normalized();
        }
        
        if (onGround) {
            float friction = 6.0f;
            float drop = velocity.Length() * friction * dt;
            float newSpeed = velocity.Length() - drop;
            if (newSpeed < 0) newSpeed = 0;
            if (velocity.LengthSq() > 0.0001f) {
                velocity = velocity.Normalized() * newSpeed;
            }
            
            float accel = 10.0f;
            float maxSpeed = speed;
            float currentSpeed = velocity.x * wishDir.x + velocity.z * wishDir.z;
            float addSpeed = maxSpeed - currentSpeed;
            if (addSpeed > 0) {
                float accelSpeed = accel * dt * maxSpeed;
                if (accelSpeed > addSpeed) accelSpeed = addSpeed;
                velocity.x += wishDir.x * accelSpeed;
                velocity.z += wishDir.z * accelSpeed;
            }
            
            if (input.HasButton(PlayerInput::Jump)) {
                velocity.y = 5.0f;
                onGround = false;
            }
        } else {
            float airAccel = 2.0f;
            float currentSpeed = velocity.x * wishDir.x + velocity.z * wishDir.z;
            float addSpeed = speed - currentSpeed;
            if (addSpeed > 0) {
                float accelSpeed = airAccel * dt * speed;
                if (accelSpeed > addSpeed) accelSpeed = addSpeed;
                velocity.x += wishDir.x * accelSpeed;
                velocity.z += wishDir.z * accelSpeed;
            }
            
            velocity.y -= 15.0f * dt;
        }
        
        position = position + velocity * dt;
        
        if (position.y < 1.0f) {
            position.y = 1.0f;
            velocity.y = 0;
            onGround = true;
        }
        
        yaw = input.yaw;
        pitch = input.pitch;
    }
};

struct WorldSnapshot {
    uint32_t tick = 0;
    float serverTime = 0;
    PlayerState players[MAX_PLAYERS];
    uint8_t playerCount = 0;
};

struct InputCommand {
    PlayerInput input;
    PlayerState predictedState;
};

class SnapshotBuffer {
public:
    void Push(const WorldSnapshot& snap) {
        if (m_buffer.size() >= SNAPSHOT_BUFFER_SIZE) {
            m_buffer.pop_front();
        }
        m_buffer.push_back(snap);
    }
    
    bool GetInterpolated(float renderTime, WorldSnapshot& out) const {
        if (m_buffer.size() < 2) return false;
        
        const WorldSnapshot* from = nullptr;
        const WorldSnapshot* to = nullptr;
        
        for (size_t i = 0; i < m_buffer.size() - 1; i++) {
            if (m_buffer[i].serverTime <= renderTime && m_buffer[i + 1].serverTime >= renderTime) {
                from = &m_buffer[i];
                to = &m_buffer[i + 1];
                break;
            }
        }
        
        if (!from || !to) {
            if (!m_buffer.empty()) {
                out = m_buffer.back();
                return true;
            }
            return false;
        }
        
        float t = 0;
        float dt = to->serverTime - from->serverTime;
        if (dt > 0.0001f) {
            t = (renderTime - from->serverTime) / dt;
            if (t < 0) t = 0;
            if (t > 1) t = 1;
        }
        
        out.tick = to->tick;
        out.serverTime = renderTime;
        out.playerCount = to->playerCount;
        
        for (int i = 0; i < MAX_PLAYERS; i++) {
            const PlayerState& a = from->players[i];
            const PlayerState& b = to->players[i];
            PlayerState& r = out.players[i];
            
            r.clientId = b.clientId;
            r.lastProcessedInput = b.lastProcessedInput;
            r.position = Vec3::Lerp(a.position, b.position, t);
            r.velocity = Vec3::Lerp(a.velocity, b.velocity, t);
            r.yaw = a.yaw + (b.yaw - a.yaw) * t;
            r.pitch = a.pitch + (b.pitch - a.pitch) * t;
            r.onGround = b.onGround;
            r.alive = b.alive;
            r.health = b.health;
        }
        
        return true;
    }
    
    float GetLatestTime() const {
        return m_buffer.empty() ? 0 : m_buffer.back().serverTime;
    }
    
    uint32_t GetLatestTick() const {
        return m_buffer.empty() ? 0 : m_buffer.back().tick;
    }
    
    void Clear() { m_buffer.clear(); }
    
private:
    std::deque<WorldSnapshot> m_buffer;
};

class InputBuffer {
public:
    void Push(const InputCommand& cmd) {
        if (m_buffer.size() >= INPUT_BUFFER_SIZE) {
            m_buffer.pop_front();
        }
        m_buffer.push_back(cmd);
    }
    
    void RemoveUpTo(uint32_t sequence) {
        while (!m_buffer.empty() && m_buffer.front().input.sequence <= sequence) {
            m_buffer.pop_front();
        }
    }
    
    const std::deque<InputCommand>& GetBuffer() const { return m_buffer; }
    
    void Clear() { m_buffer.clear(); }
    
private:
    std::deque<InputCommand> m_buffer;
};

inline void WritePlayerInput(uint8_t* data, uint16_t& size, const PlayerInput& input) {
    memcpy(data + size, &input.tick, 4); size += 4;
    memcpy(data + size, &input.sequence, 4); size += 4;
    memcpy(data + size, &input.buttons, 1); size += 1;
    memcpy(data + size, &input.moveX, 4); size += 4;
    memcpy(data + size, &input.moveY, 4); size += 4;
    memcpy(data + size, &input.yaw, 4); size += 4;
    memcpy(data + size, &input.pitch, 4); size += 4;
}

inline void ReadPlayerInput(const uint8_t* data, uint16_t& pos, PlayerInput& input) {
    memcpy(&input.tick, data + pos, 4); pos += 4;
    memcpy(&input.sequence, data + pos, 4); pos += 4;
    memcpy(&input.buttons, data + pos, 1); pos += 1;
    memcpy(&input.moveX, data + pos, 4); pos += 4;
    memcpy(&input.moveY, data + pos, 4); pos += 4;
    memcpy(&input.yaw, data + pos, 4); pos += 4;
    memcpy(&input.pitch, data + pos, 4); pos += 4;
}

inline void WritePlayerState(uint8_t* data, uint16_t& size, const PlayerState& state) {
    memcpy(data + size, &state.clientId, 4); size += 4;
    memcpy(data + size, &state.lastProcessedInput, 4); size += 4;
    memcpy(data + size, &state.position.x, 4); size += 4;
    memcpy(data + size, &state.position.y, 4); size += 4;
    memcpy(data + size, &state.position.z, 4); size += 4;
    memcpy(data + size, &state.velocity.x, 4); size += 4;
    memcpy(data + size, &state.velocity.y, 4); size += 4;
    memcpy(data + size, &state.velocity.z, 4); size += 4;
    memcpy(data + size, &state.yaw, 4); size += 4;
    memcpy(data + size, &state.pitch, 4); size += 4;
    uint8_t flags = (state.onGround ? 1 : 0) | (state.alive ? 2 : 0);
    memcpy(data + size, &flags, 1); size += 1;
    memcpy(data + size, &state.health, 1); size += 1;
}

inline void ReadPlayerState(const uint8_t* data, uint16_t& pos, PlayerState& state) {
    memcpy(&state.clientId, data + pos, 4); pos += 4;
    memcpy(&state.lastProcessedInput, data + pos, 4); pos += 4;
    memcpy(&state.position.x, data + pos, 4); pos += 4;
    memcpy(&state.position.y, data + pos, 4); pos += 4;
    memcpy(&state.position.z, data + pos, 4); pos += 4;
    memcpy(&state.velocity.x, data + pos, 4); pos += 4;
    memcpy(&state.velocity.y, data + pos, 4); pos += 4;
    memcpy(&state.velocity.z, data + pos, 4); pos += 4;
    memcpy(&state.yaw, data + pos, 4); pos += 4;
    memcpy(&state.pitch, data + pos, 4); pos += 4;
    uint8_t flags; memcpy(&flags, data + pos, 1); pos += 1;
    state.onGround = (flags & 1) != 0;
    state.alive = (flags & 2) != 0;
    memcpy(&state.health, data + pos, 1); pos += 1;
}

}
}

#endif

