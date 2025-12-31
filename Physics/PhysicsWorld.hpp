#ifndef TITAN_PHYSICS_WORLD_HPP
#define TITAN_PHYSICS_WORLD_HPP

#include <cstdint>
#include <cmath>
#include <vector>
#include <functional>
#include <algorithm>

namespace Titan {
namespace Physics {

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 Cross(const Vec3& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float LengthSq() const { return x * x + y * y + z * z; }
    Vec3 Normalized() const { float l = Length(); return l > 0.0001f ? Vec3{x/l, y/l, z/l} : *this; }
    static Vec3 Min(const Vec3& a, const Vec3& b) { return {fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)}; }
    static Vec3 Max(const Vec3& a, const Vec3& b) { return {fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)}; }
    static Vec3 Lerp(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }
};

struct AABB {
    Vec3 min, max;
    
    AABB() : min{0,0,0}, max{0,0,0} {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    
    static AABB FromCenterSize(const Vec3& center, const Vec3& size) {
        Vec3 half = size * 0.5f;
        return {center - half, center + half};
    }
    
    Vec3 Center() const { return (min + max) * 0.5f; }
    Vec3 Size() const { return max - min; }
    Vec3 Extents() const { return (max - min) * 0.5f; }
    
    bool Contains(const Vec3& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }
    
    bool Intersects(const AABB& o) const {
        return min.x <= o.max.x && max.x >= o.min.x &&
               min.y <= o.max.y && max.y >= o.min.y &&
               min.z <= o.max.z && max.z >= o.min.z;
    }
    
    void Expand(const Vec3& p) {
        min = Vec3::Min(min, p);
        max = Vec3::Max(max, p);
    }
};

struct Sphere {
    Vec3 center;
    float radius = 0.5f;
    
    bool Contains(const Vec3& p) const {
        return (p - center).LengthSq() <= radius * radius;
    }
    
    bool Intersects(const Sphere& o) const {
        float d = (center - o.center).Length();
        return d <= radius + o.radius;
    }
};

struct Capsule {
    Vec3 a, b;
    float radius = 0.3f;
    
    Vec3 ClosestPoint(const Vec3& p) const {
        Vec3 ab = b - a;
        float t = (p - a).Dot(ab) / ab.Dot(ab);
        t = fmaxf(0, fminf(1, t));
        return a + ab * t;
    }
};

struct Plane {
    Vec3 normal;
    float distance = 0;
    
    Plane() : normal{0, 1, 0}, distance(0) {}
    Plane(const Vec3& n, float d) : normal(n.Normalized()), distance(d) {}
    
    float DistanceTo(const Vec3& p) const {
        return normal.Dot(p) - distance;
    }
    
    Vec3 ClosestPoint(const Vec3& p) const {
        return p - normal * DistanceTo(p);
    }
};

struct RaycastHit {
    bool hit = false;
    float distance = 0;
Vec3 point;
Vec3 normal;
    uint32_t colliderId = 0;
};

enum class ColliderType {
    None,
    Box,
    Sphere,
    Capsule,
    Mesh
};

struct Collider {
    uint32_t id = 0;
    ColliderType type = ColliderType::None;
    Vec3 center;
    Vec3 size = {1, 1, 1};
    float radius = 0.5f;
    float height = 2.0f;
    bool isTrigger = false;
    bool isStatic = true;
    uint32_t layer = 0xFFFFFFFF;
    void* userData = nullptr;
};

struct Rigidbody {
    uint32_t id = 0;
    Vec3 position;
    Vec3 velocity;
    Vec3 angularVelocity;
    float mass = 1.0f;
    float drag = 0.1f;
    float angularDrag = 0.05f;
    bool useGravity = true;
    bool isKinematic = false;
    bool freezeRotation = false;
    uint32_t colliderId = 0;
};

struct CollisionInfo {
    uint32_t colliderA = 0;
    uint32_t colliderB = 0;
    Vec3 contactPoint;
    Vec3 normal;
    float penetration = 0;
};

class PhysicsWorld {
public:
    Vec3 gravity = {0, -9.81f, 0};
    float fixedDeltaTime = 1.0f / 60.0f;
    int maxIterations = 4;
    
    void Init() {
        m_accumulator = 0;
        m_nextColliderId = 1;
        m_nextRigidbodyId = 1;
        printf("[Physics] World initialized\n");
    }
    
    void Update(float dt) {
        m_accumulator += dt;
        
        int iterations = 0;
        while (m_accumulator >= fixedDeltaTime && iterations < maxIterations) {
            Step(fixedDeltaTime);
            m_accumulator -= fixedDeltaTime;
            iterations++;
        }
    }
    
    uint32_t CreateBoxCollider(const Vec3& center, const Vec3& size, bool isStatic = true) {
        Collider c;
        c.id = m_nextColliderId++;
        c.type = ColliderType::Box;
        c.center = center;
        c.size = size;
        c.isStatic = isStatic;
        m_colliders.push_back(c);
        return c.id;
    }
    
    uint32_t CreateSphereCollider(const Vec3& center, float radius, bool isStatic = true) {
        Collider c;
        c.id = m_nextColliderId++;
        c.type = ColliderType::Sphere;
        c.center = center;
        c.radius = radius;
        c.isStatic = isStatic;
        m_colliders.push_back(c);
        return c.id;
    }
    
    uint32_t CreateCapsuleCollider(const Vec3& center, float radius, float height, bool isStatic = true) {
        Collider c;
        c.id = m_nextColliderId++;
        c.type = ColliderType::Capsule;
        c.center = center;
        c.radius = radius;
        c.height = height;
        c.isStatic = isStatic;
        m_colliders.push_back(c);
        return c.id;
    }
    
    uint32_t CreateRigidbody(const Vec3& position, float mass = 1.0f) {
        Rigidbody rb;
        rb.id = m_nextRigidbodyId++;
        rb.position = position;
        rb.mass = mass;
        m_rigidbodies.push_back(rb);
        return rb.id;
    }
    
    void SetColliderPosition(uint32_t id, const Vec3& pos) {
        for (auto& c : m_colliders) {
            if (c.id == id) { c.center = pos; return; }
        }
    }
    
    void SetRigidbodyPosition(uint32_t id, const Vec3& pos) {
        for (auto& rb : m_rigidbodies) {
            if (rb.id == id) { rb.position = pos; return; }
        }
    }
    
    void SetRigidbodyVelocity(uint32_t id, const Vec3& vel) {
        for (auto& rb : m_rigidbodies) {
            if (rb.id == id) { rb.velocity = vel; return; }
        }
    }
    
    void AddForce(uint32_t id, const Vec3& force) {
        for (auto& rb : m_rigidbodies) {
            if (rb.id == id && rb.mass > 0.0001f) {
                rb.velocity = rb.velocity + force * (1.0f / rb.mass);
                return;
            }
        }
    }
    
    void AddImpulse(uint32_t id, const Vec3& impulse) {
        for (auto& rb : m_rigidbodies) {
            if (rb.id == id && rb.mass > 0.0001f) {
                rb.velocity = rb.velocity + impulse * (1.0f / rb.mass);
                return;
            }
        }
    }
    
    RaycastHit Raycast(const Vec3& origin, const Vec3& direction, float maxDistance = 1000.0f, uint32_t layerMask = 0xFFFFFFFF) {
        RaycastHit result;
        result.distance = maxDistance;
        
        Vec3 dir = direction.Normalized();
        
        for (const auto& c : m_colliders) {
            if (!(c.layer & layerMask)) continue;
            
            RaycastHit hit;
            
            switch (c.type) {
                case ColliderType::Box:
                    hit = RaycastBox(origin, dir, c);
                    break;
                case ColliderType::Sphere:
                    hit = RaycastSphere(origin, dir, c);
                    break;
                case ColliderType::Capsule:
                    hit = RaycastCapsule(origin, dir, c);
                    break;
                default:
                    continue;
            }
            
            if (hit.hit && hit.distance < result.distance) {
                result = hit;
                result.colliderId = c.id;
            }
        }
    
    return result;
}

    bool CheckSphere(const Vec3& center, float radius, uint32_t layerMask = 0xFFFFFFFF) {
        for (const auto& c : m_colliders) {
            if (!(c.layer & layerMask)) continue;
            if (SphereIntersectsCollider(center, radius, c)) return true;
        }
        return false;
    }
    
    bool CheckBox(const Vec3& center, const Vec3& size, uint32_t layerMask = 0xFFFFFFFF) {
        AABB box = AABB::FromCenterSize(center, size);
        for (const auto& c : m_colliders) {
            if (!(c.layer & layerMask)) continue;
            if (BoxIntersectsCollider(box, c)) return true;
        }
        return false;
    }
    
    std::vector<uint32_t> OverlapSphere(const Vec3& center, float radius, uint32_t layerMask = 0xFFFFFFFF) {
        std::vector<uint32_t> result;
        for (const auto& c : m_colliders) {
            if (!(c.layer & layerMask)) continue;
            if (SphereIntersectsCollider(center, radius, c)) {
                result.push_back(c.id);
            }
        }
        return result;
    }
    
    Vec3 SweepCapsule(const Vec3& start, const Vec3& end, float radius, float height, Vec3& hitNormal) {
        Vec3 result = end;
        hitNormal = {0, 0, 0};
        
        Vec3 dir = end - start;
        float dist = dir.Length();
        if (dist < 0.0001f) return start;
        
        dir = dir.Normalized();
        
        int steps = (int)(dist / 0.05f) + 1;
        float stepSize = dist / steps;
        
        Vec3 current = start;
        
        for (int i = 0; i <= steps; i++) {
            Vec3 testPos = start + dir * (stepSize * i);
            
            for (const auto& c : m_colliders) {
                if (!c.isStatic) continue;
                
                Vec3 pushOut;
                if (CapsuleIntersectsCollider(testPos, radius, height, c, pushOut)) {
                    hitNormal = pushOut.Normalized();
                    result = testPos + pushOut;
                    
                    Vec3 remaining = end - result;
                    float slide = remaining.Dot(hitNormal);
                    result = result + (remaining - hitNormal * slide);
                    
                    return result;
                }
            }
            
            current = testPos;
        }
        
        return end;
    }
    
    void RemoveCollider(uint32_t id) {
        m_colliders.erase(
            std::remove_if(m_colliders.begin(), m_colliders.end(),
                [id](const Collider& c) { return c.id == id; }),
            m_colliders.end()
        );
    }
    
    void RemoveRigidbody(uint32_t id) {
        m_rigidbodies.erase(
            std::remove_if(m_rigidbodies.begin(), m_rigidbodies.end(),
                [id](const Rigidbody& rb) { return rb.id == id; }),
            m_rigidbodies.end()
        );
    }
    
    void Clear() {
        m_colliders.clear();
        m_rigidbodies.clear();
    }
    
    Rigidbody* GetRigidbody(uint32_t id) {
        for (auto& rb : m_rigidbodies) {
            if (rb.id == id) return &rb;
        }
        return nullptr;
    }
    
    Collider* GetCollider(uint32_t id) {
        for (auto& c : m_colliders) {
            if (c.id == id) return &c;
        }
        return nullptr;
    }
    
    const std::vector<Collider>& GetColliders() const { return m_colliders; }
    const std::vector<Rigidbody>& GetRigidbodies() const { return m_rigidbodies; }

private:
    void Step(float dt) {
        for (auto& rb : m_rigidbodies) {
            if (rb.isKinematic) continue;
            
            if (rb.useGravity) {
                rb.velocity = rb.velocity + gravity * dt;
            }
            
            rb.velocity = rb.velocity * (1.0f - rb.drag * dt);
            rb.position = rb.position + rb.velocity * dt;
            
            Collider* col = rb.colliderId ? GetCollider(rb.colliderId) : nullptr;
            if (col) {
                col->center = rb.position;
            }
        }
        
        DetectCollisions();
        ResolveCollisions();
}

void DetectCollisions() {
        m_contacts.clear();
        
        for (size_t i = 0; i < m_colliders.size(); i++) {
            for (size_t j = i + 1; j < m_colliders.size(); j++) {
                Collider& a = m_colliders[i];
                Collider& b = m_colliders[j];
                
                if (a.isStatic && b.isStatic) continue;
                
                CollisionInfo info;
                if (TestCollision(a, b, info)) {
                    m_contacts.push_back(info);
                }
            }
        }
    }
    
    void ResolveCollisions() {
        for (const auto& contact : m_contacts) {
            Collider* a = GetCollider(contact.colliderA);
            Collider* b = GetCollider(contact.colliderB);
            if (!a || !b) continue;
            
            if (!a->isStatic) {
                a->center = a->center + contact.normal * contact.penetration * 0.5f;
            }
            if (!b->isStatic) {
                b->center = b->center - contact.normal * contact.penetration * 0.5f;
            }
        }
    }
    
    bool TestCollision(const Collider& a, const Collider& b, CollisionInfo& info) {
        if (a.type == ColliderType::Sphere && b.type == ColliderType::Sphere) {
            return SphereSphereTest(a, b, info);
        }
        if (a.type == ColliderType::Box && b.type == ColliderType::Box) {
            return BoxBoxTest(a, b, info);
        }
        if (a.type == ColliderType::Sphere && b.type == ColliderType::Box) {
            return SphereBoxTest(a, b, info);
        }
        if (a.type == ColliderType::Box && b.type == ColliderType::Sphere) {
            bool result = SphereBoxTest(b, a, info);
            if (result) {
                std::swap(info.colliderA, info.colliderB);
                info.normal = -info.normal;
            }
            return result;
        }
        return false;
    }
    
    bool SphereSphereTest(const Collider& a, const Collider& b, CollisionInfo& info) {
        Vec3 diff = b.center - a.center;
        float dist = diff.Length();
        float sumRadii = a.radius + b.radius;
        
        if (dist < sumRadii) {
            info.colliderA = a.id;
            info.colliderB = b.id;
            info.penetration = sumRadii - dist;
            info.normal = dist > 0.0001f ? diff.Normalized() : Vec3{0, 1, 0};
            info.contactPoint = a.center + info.normal * a.radius;
            return true;
        }
        return false;
    }
    
    bool BoxBoxTest(const Collider& a, const Collider& b, CollisionInfo& info) {
        AABB boxA = AABB::FromCenterSize(a.center, a.size);
        AABB boxB = AABB::FromCenterSize(b.center, b.size);
        
        if (!boxA.Intersects(boxB)) return false;
        
        Vec3 overlap;
        overlap.x = fminf(boxA.max.x - boxB.min.x, boxB.max.x - boxA.min.x);
        overlap.y = fminf(boxA.max.y - boxB.min.y, boxB.max.y - boxA.min.y);
        overlap.z = fminf(boxA.max.z - boxB.min.z, boxB.max.z - boxA.min.z);
        
        info.colliderA = a.id;
        info.colliderB = b.id;
        
        if (overlap.x < overlap.y && overlap.x < overlap.z) {
            info.penetration = overlap.x;
            info.normal = a.center.x < b.center.x ? Vec3{-1, 0, 0} : Vec3{1, 0, 0};
        } else if (overlap.y < overlap.z) {
            info.penetration = overlap.y;
            info.normal = a.center.y < b.center.y ? Vec3{0, -1, 0} : Vec3{0, 1, 0};
        } else {
            info.penetration = overlap.z;
            info.normal = a.center.z < b.center.z ? Vec3{0, 0, -1} : Vec3{0, 0, 1};
        }
        
        info.contactPoint = a.center + info.normal * (a.size.x * 0.5f);
        return true;
    }
    
    bool SphereBoxTest(const Collider& sphere, const Collider& box, CollisionInfo& info) {
        AABB aabb = AABB::FromCenterSize(box.center, box.size);
        
        Vec3 closest;
        closest.x = fmaxf(aabb.min.x, fminf(sphere.center.x, aabb.max.x));
        closest.y = fmaxf(aabb.min.y, fminf(sphere.center.y, aabb.max.y));
        closest.z = fmaxf(aabb.min.z, fminf(sphere.center.z, aabb.max.z));
        
        Vec3 diff = sphere.center - closest;
        float dist = diff.Length();
        
        if (dist < sphere.radius) {
            info.colliderA = sphere.id;
            info.colliderB = box.id;
            info.penetration = sphere.radius - dist;
            info.normal = dist > 0.0001f ? diff.Normalized() : Vec3{0, 1, 0};
            info.contactPoint = closest;
            return true;
        }
        return false;
    }
    
    RaycastHit RaycastBox(const Vec3& origin, const Vec3& dir, const Collider& c) {
        RaycastHit hit;
        AABB box = AABB::FromCenterSize(c.center, c.size);
        
        float tmin = -1e30f, tmax = 1e30f;
        Vec3 tminNormal, tmaxNormal;
        
        for (int i = 0; i < 3; i++) {
            float o = (&origin.x)[i];
            float d = (&dir.x)[i];
            float bmin = (&box.min.x)[i];
            float bmax = (&box.max.x)[i];
            
            if (fabsf(d) < 0.0001f) {
                if (o < bmin || o > bmax) return hit;
            } else {
                float t1 = (bmin - o) / d;
                float t2 = (bmax - o) / d;
                
                Vec3 n1 = {0, 0, 0};
                Vec3 n2 = {0, 0, 0};
                (&n1.x)[i] = -1;
                (&n2.x)[i] = 1;
                
                if (t1 > t2) { std::swap(t1, t2); std::swap(n1, n2); }
                
                if (t1 > tmin) { tmin = t1; tminNormal = n1; }
                if (t2 < tmax) { tmax = t2; tmaxNormal = n2; }
                
                if (tmin > tmax) return hit;
            }
        }
        
        if (tmin > 0) {
            hit.hit = true;
            hit.distance = tmin;
            hit.point = origin + dir * tmin;
            hit.normal = tminNormal;
        }
        
        return hit;
    }
    
    RaycastHit RaycastSphere(const Vec3& origin, const Vec3& dir, const Collider& c) {
        RaycastHit hit;
        
        Vec3 oc = origin - c.center;
        float a = dir.Dot(dir);
        float b = 2.0f * oc.Dot(dir);
        float cc = oc.Dot(oc) - c.radius * c.radius;
        float discriminant = b * b - 4 * a * cc;
        
        if (discriminant < 0) return hit;
        
        float t = (-b - sqrtf(discriminant)) / (2 * a);
        if (t < 0) t = (-b + sqrtf(discriminant)) / (2 * a);
        
        if (t > 0) {
            hit.hit = true;
            hit.distance = t;
            hit.point = origin + dir * t;
            hit.normal = (hit.point - c.center).Normalized();
        }
        
        return hit;
    }
    
    RaycastHit RaycastCapsule(const Vec3& origin, const Vec3& dir, const Collider& c) {
        float halfHeight = (c.height - 2 * c.radius) * 0.5f;
        Vec3 a = c.center + Vec3{0, halfHeight, 0};
        Vec3 b = c.center - Vec3{0, halfHeight, 0};
        
        RaycastHit bestHit;
        bestHit.distance = 1e30f;
        
        Collider sphereA;
        sphereA.center = a;
        sphereA.radius = c.radius;
        RaycastHit hitA = RaycastSphere(origin, dir, sphereA);
        if (hitA.hit && hitA.distance < bestHit.distance) bestHit = hitA;
        
        Collider sphereB;
        sphereB.center = b;
        sphereB.radius = c.radius;
        RaycastHit hitB = RaycastSphere(origin, dir, sphereB);
        if (hitB.hit && hitB.distance < bestHit.distance) bestHit = hitB;
        
        return bestHit.hit ? bestHit : RaycastHit{};
    }
    
    bool SphereIntersectsCollider(const Vec3& center, float radius, const Collider& c) {
        switch (c.type) {
            case ColliderType::Sphere: {
                float dist = (center - c.center).Length();
                return dist < radius + c.radius;
            }
            case ColliderType::Box: {
                AABB box = AABB::FromCenterSize(c.center, c.size);
                Vec3 closest;
                closest.x = fmaxf(box.min.x, fminf(center.x, box.max.x));
                closest.y = fmaxf(box.min.y, fminf(center.y, box.max.y));
                closest.z = fmaxf(box.min.z, fminf(center.z, box.max.z));
                return (center - closest).LengthSq() < radius * radius;
            }
            default:
                return false;
        }
    }
    
    bool BoxIntersectsCollider(const AABB& box, const Collider& c) {
        switch (c.type) {
            case ColliderType::Box: {
                AABB other = AABB::FromCenterSize(c.center, c.size);
                return box.Intersects(other);
            }
            case ColliderType::Sphere: {
                Vec3 closest;
                closest.x = fmaxf(box.min.x, fminf(c.center.x, box.max.x));
                closest.y = fmaxf(box.min.y, fminf(c.center.y, box.max.y));
                closest.z = fmaxf(box.min.z, fminf(c.center.z, box.max.z));
                return (c.center - closest).LengthSq() < c.radius * c.radius;
            }
            default:
                return false;
        }
    }
    
    bool CapsuleIntersectsCollider(const Vec3& center, float radius, float height, const Collider& c, Vec3& pushOut) {
        float halfHeight = (height - 2 * radius) * 0.5f;
        Vec3 top = center + Vec3{0, halfHeight, 0};
        Vec3 bottom = center - Vec3{0, halfHeight, 0};
        
        switch (c.type) {
            case ColliderType::Box: {
                AABB box = AABB::FromCenterSize(c.center, c.size);
                
                Vec3 closest;
                closest.x = fmaxf(box.min.x, fminf(center.x, box.max.x));
                closest.y = fmaxf(box.min.y, fminf(center.y, box.max.y));
                closest.z = fmaxf(box.min.z, fminf(center.z, box.max.z));
                
                Vec3 diff = center - closest;
                float dist = diff.Length();
                
                if (dist < radius) {
                    if (dist > 0.0001f) {
                        pushOut = diff.Normalized() * (radius - dist);
                    } else {
                        pushOut = {0, radius, 0};
                    }
                    return true;
                }
                return false;
            }
            case ColliderType::Sphere: {
                Vec3 diff = center - c.center;
                float dist = diff.Length();
                float sumRadii = radius + c.radius;
                
                if (dist < sumRadii) {
                    if (dist > 0.0001f) {
                        pushOut = diff.Normalized() * (sumRadii - dist);
                    } else {
                        pushOut = {0, sumRadii, 0};
                    }
                    return true;
                }
                return false;
            }
            default:
                return false;
        }
    }
    
    std::vector<Collider> m_colliders;
    std::vector<Rigidbody> m_rigidbodies;
    std::vector<CollisionInfo> m_contacts;
    
    uint32_t m_nextColliderId = 1;
    uint32_t m_nextRigidbodyId = 1;
    float m_accumulator = 0;
};

}
}

#endif
