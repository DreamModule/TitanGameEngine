#ifndef TITAN_LAG_COMPENSATION_HPP
#define TITAN_LAG_COMPENSATION_HPP

#include <cstdint>
#include <cmath>
#include <vector>
#include <deque>
#include <algorithm>

namespace Titan {
namespace Net {

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
    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    static Vec3 Lerp(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }
};

struct Hitbox {
    Vec3 center;
    Vec3 halfSize;
    uint32_t boneId = 0;
    float damageMultiplier = 1.0f;
    bool isHead = false;
};

struct PlayerHitboxes {
    uint32_t clientId = 0;
    std::vector<Hitbox> hitboxes;
    Vec3 position;
    float yaw = 0;
    float pitch = 0;
    
    void SetupDefault() {
        hitboxes.clear();
        
        Hitbox body;
        body.center = {0, 0.9f, 0};
        body.halfSize = {0.35f, 0.5f, 0.2f};
        body.damageMultiplier = 1.0f;
        body.isHead = false;
        hitboxes.push_back(body);
        
        Hitbox head;
        head.center = {0, 1.6f, 0};
        head.halfSize = {0.15f, 0.15f, 0.15f};
        head.damageMultiplier = 4.0f;
        head.isHead = true;
        hitboxes.push_back(head);
        
        Hitbox leftArm;
        leftArm.center = {-0.45f, 0.9f, 0};
        leftArm.halfSize = {0.1f, 0.35f, 0.1f};
        leftArm.damageMultiplier = 0.75f;
        hitboxes.push_back(leftArm);
        
        Hitbox rightArm;
        rightArm.center = {0.45f, 0.9f, 0};
        rightArm.halfSize = {0.1f, 0.35f, 0.1f};
        rightArm.damageMultiplier = 0.75f;
        hitboxes.push_back(rightArm);
        
        Hitbox leftLeg;
        leftLeg.center = {-0.15f, 0.25f, 0};
        leftLeg.halfSize = {0.1f, 0.4f, 0.1f};
        leftLeg.damageMultiplier = 0.75f;
        hitboxes.push_back(leftLeg);
        
        Hitbox rightLeg;
        rightLeg.center = {0.15f, 0.25f, 0};
        rightLeg.halfSize = {0.1f, 0.4f, 0.1f};
        rightLeg.damageMultiplier = 0.75f;
        hitboxes.push_back(rightLeg);
    }
    
    Hitbox GetWorldHitbox(size_t index) const {
        if (index >= hitboxes.size()) return Hitbox();
        
        Hitbox world = hitboxes[index];
        
        float cosY = cosf(yaw);
        float sinY = sinf(yaw);
        
        Vec3 rotated;
        rotated.x = world.center.x * cosY - world.center.z * sinY;
        rotated.y = world.center.y;
        rotated.z = world.center.x * sinY + world.center.z * cosY;
        
        world.center = position + rotated;
        return world;
    }
};

struct HistoryFrame {
    uint32_t tick = 0;
    float serverTime = 0;
    std::vector<PlayerHitboxes> players;
};

struct RaycastResult {
    bool hit = false;
    float distance = 0;
    Vec3 point;
    Vec3 normal;
    uint32_t clientId = 0;
    int hitboxIndex = -1;
    float damageMultiplier = 1.0f;
    bool isHeadshot = false;
};

class LagCompensation {
public:
    float maxHistoryTime = 1.0f;
    int maxHistoryFrames = 128;
    float maxLagCompensation = 0.5f;
    
    void Init() {
        m_history.clear();
        printf("[LagComp] Lag compensation initialized (max %.0fms)\n", maxLagCompensation * 1000);
    }
    
    void RecordFrame(uint32_t tick, float serverTime, const std::vector<PlayerHitboxes>& players) {
        HistoryFrame frame;
        frame.tick = tick;
        frame.serverTime = serverTime;
        frame.players = players;
        
        m_history.push_back(frame);
        
        while (m_history.size() > (size_t)maxHistoryFrames) {
            m_history.pop_front();
        }
        
        while (!m_history.empty() && serverTime - m_history.front().serverTime > maxHistoryTime) {
            m_history.pop_front();
        }
    }
    
    bool GetHistoryAtTime(float targetTime, HistoryFrame& outFrame) {
        if (m_history.empty()) return false;
        
        if (targetTime <= m_history.front().serverTime) {
            outFrame = m_history.front();
            return true;
        }
        
        if (targetTime >= m_history.back().serverTime) {
            outFrame = m_history.back();
            return true;
        }
        
        const HistoryFrame* from = nullptr;
        const HistoryFrame* to = nullptr;
        
        for (size_t i = 0; i < m_history.size() - 1; i++) {
            if (m_history[i].serverTime <= targetTime && m_history[i + 1].serverTime >= targetTime) {
                from = &m_history[i];
                to = &m_history[i + 1];
                break;
            }
        }
        
        if (!from || !to) {
            outFrame = m_history.back();
            return true;
        }
        
        float t = 0;
        float dt = to->serverTime - from->serverTime;
        if (dt > 0.0001f) {
            t = (targetTime - from->serverTime) / dt;
        }
        
        outFrame.tick = to->tick;
        outFrame.serverTime = targetTime;
        
        for (size_t i = 0; i < to->players.size(); i++) {
            PlayerHitboxes interp = to->players[i];
            
            for (size_t j = 0; j < from->players.size(); j++) {
                if (from->players[j].clientId == interp.clientId) {
                    const PlayerHitboxes& p1 = from->players[j];
                    const PlayerHitboxes& p2 = to->players[i];
                    
                    interp.position = Vec3::Lerp(p1.position, p2.position, t);
                    interp.yaw = p1.yaw + (p2.yaw - p1.yaw) * t;
                    interp.pitch = p1.pitch + (p2.pitch - p1.pitch) * t;
                    break;
                }
            }
            
            outFrame.players.push_back(interp);
        }
        
        return true;
    }
    
    RaycastResult RaycastAtTime(
        float targetTime,
        const Vec3& origin,
        const Vec3& direction,
        float maxDistance,
        uint32_t shooterClientId
    ) {
        RaycastResult result;
        
        targetTime = fmaxf(targetTime, m_history.empty() ? 0 : m_history.back().serverTime - maxLagCompensation);
        
        HistoryFrame frame;
        if (!GetHistoryAtTime(targetTime, frame)) {
            return result;
        }
        
        Vec3 dir = direction.Normalized();
        float closestDist = maxDistance;
        
        for (const auto& player : frame.players) {
            if (player.clientId == shooterClientId) continue;
            
            for (size_t i = 0; i < player.hitboxes.size(); i++) {
                Hitbox worldHB = player.GetWorldHitbox(i);
                
                float hitDist;
                Vec3 hitPoint, hitNormal;
                
                if (RaycastAABB(origin, dir, worldHB.center, worldHB.halfSize, hitDist, hitPoint, hitNormal)) {
                    if (hitDist < closestDist && hitDist > 0) {
                        closestDist = hitDist;
                        result.hit = true;
                        result.distance = hitDist;
                        result.point = hitPoint;
                        result.normal = hitNormal;
                        result.clientId = player.clientId;
                        result.hitboxIndex = (int)i;
                        result.damageMultiplier = worldHB.damageMultiplier;
                        result.isHeadshot = worldHB.isHead;
                    }
                }
            }
        }
        
        return result;
    }
    
    RaycastResult Raycast(
        const Vec3& origin,
        const Vec3& direction,
        float maxDistance,
        uint32_t shooterClientId
    ) {
        if (m_history.empty()) return RaycastResult();
        return RaycastAtTime(m_history.back().serverTime, origin, direction, maxDistance, shooterClientId);
    }
    
    void Clear() {
        m_history.clear();
    }
    
    size_t GetHistorySize() const { return m_history.size(); }
    
    float GetOldestTime() const {
        return m_history.empty() ? 0 : m_history.front().serverTime;
    }
    
    float GetNewestTime() const {
        return m_history.empty() ? 0 : m_history.back().serverTime;
    }

private:
    bool RaycastAABB(
        const Vec3& origin,
        const Vec3& dir,
        const Vec3& center,
        const Vec3& halfSize,
        float& outDist,
        Vec3& outPoint,
        Vec3& outNormal
    ) {
        Vec3 bmin = center - Vec3{halfSize.x, halfSize.y, halfSize.z};
        Vec3 bmax = center + Vec3{halfSize.x, halfSize.y, halfSize.z};
        
        float tmin = -1e30f, tmax = 1e30f;
        int axis = 0;
        
        for (int i = 0; i < 3; i++) {
            float o = (&origin.x)[i];
            float d = (&dir.x)[i];
            float mn = (&bmin.x)[i];
            float mx = (&bmax.x)[i];
            
            if (fabsf(d) < 0.0001f) {
                if (o < mn || o > mx) return false;
            } else {
                float t1 = (mn - o) / d;
                float t2 = (mx - o) / d;
                
                if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
                
                if (t1 > tmin) { tmin = t1; axis = i; }
                if (t2 < tmax) { tmax = t2; }
                
                if (tmin > tmax) return false;
            }
        }
        
        if (tmin < 0) return false;
        
        outDist = tmin;
        outPoint = origin + dir * tmin;
        
        outNormal = {0, 0, 0};
        (&outNormal.x)[axis] = dir.x * (axis == 0 ? 1 : 0) + 
                                dir.y * (axis == 1 ? 1 : 0) + 
                                dir.z * (axis == 2 ? 1 : 0) > 0 ? -1.0f : 1.0f;
        
        return true;
    }
    
    std::deque<HistoryFrame> m_history;
};

class ShotValidator {
public:
    struct ShotResult {
        bool valid = false;
        bool hit = false;
        uint32_t victimId = 0;
        float damage = 0;
        bool isHeadshot = false;
        Vec3 hitPoint;
        Vec3 hitNormal;
        const char* invalidReason = nullptr;
    };
    
    float maxPlayerSpeed = 10.0f;
    float maxWeaponRange = 500.0f;
    float maxPingAllowance = 0.3f;
    
    ShotResult ValidateShot(
        LagCompensation& lagComp,
        float clientTime,
        float serverTime,
        float clientPing,
        uint32_t shooterClientId,
        const Vec3& origin,
        const Vec3& direction,
        float baseDamage
    ) {
        ShotResult result;
        
        float lagTime = clientPing / 1000.0f;
        if (lagTime > maxPingAllowance) {
            lagTime = maxPingAllowance;
        }
        
        float targetTime = serverTime - lagTime;
        
        if (targetTime < lagComp.GetOldestTime()) {
            result.invalidReason = "Shot too old";
            return result;
        }
        
        result.valid = true;
        
        RaycastResult rayResult = lagComp.RaycastAtTime(
            targetTime,
            origin,
            direction,
            maxWeaponRange,
            shooterClientId
        );
        
        if (rayResult.hit) {
            result.hit = true;
            result.victimId = rayResult.clientId;
            result.damage = baseDamage * rayResult.damageMultiplier;
            result.isHeadshot = rayResult.isHeadshot;
            result.hitPoint = rayResult.point;
            result.hitNormal = rayResult.normal;
        }
        
        return result;
    }
};

}
}

#endif


