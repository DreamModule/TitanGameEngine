#pragma once

#include <cmath>

namespace Titan {
namespace ECS {
namespace Components {

struct Vec3 {
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;

```
Vec3() = default;
Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 operator/(float scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
}

Vec3& operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& operator-=(const Vec3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vec3& operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

float Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float LengthSquared() const {
    return x * x + y * y + z * z;
}

Vec3 Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        return *this / len;
    }
    return Vec3(0, 0, 0);
}

void Normalize() {
    float len = Length();
    if (len > 0.0f) {
        x /= len;
        y /= len;
        z /= len;
    }
}

float Dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

static Vec3 Zero() { return Vec3(0, 0, 0); }
static Vec3 One() { return Vec3(1, 1, 1); }
static Vec3 Up() { return Vec3(0, 1, 0); }
static Vec3 Down() { return Vec3(0, -1, 0); }
static Vec3 Left() { return Vec3(-1, 0, 0); }
static Vec3 Right() { return Vec3(1, 0, 0); }
static Vec3 Forward() { return Vec3(0, 0, 1); }
static Vec3 Back() { return Vec3(0, 0, -1); }
```

};

struct Quaternion {
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;
float w = 1.0f;

```
Quaternion() = default;
Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

Quaternion operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}

Vec3 operator*(const Vec3& v) const {
    Vec3 qv(x, y, z);
    Vec3 uv = qv.Cross(v);
    Vec3 uuv = qv.Cross(uv);
    return v + (uv * (2.0f * w)) + (uuv * 2.0f);
}

static Quaternion Identity() {
    return Quaternion(0, 0, 0, 1);
}

static Quaternion FromEuler(float pitch, float yaw, float roll) {
    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);
    
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

Vec3 ToEuler() const {
    Vec3 euler;
    
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);
    
    float sinp = 2.0f * (w * y - z * x);
    if (std::abs(sinp) >= 1.0f)
        euler.y = std::copysign(3.14159265358979323846f / 2.0f, sinp);
    else
        euler.y = std::asin(sinp);
    
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);
    
    return euler;
}

Quaternion Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

float Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

Quaternion Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        return Quaternion(x / len, y / len, z / len, w / len);
    }
    return Identity();
}
```

};

struct Transform {
Vec3 position = Vec3::Zero();
Quaternion rotation = Quaternion::Identity();
Vec3 scale = Vec3::One();

```
Transform() = default;

Transform(const Vec3& pos) : position(pos) {}

Transform(const Vec3& pos, const Quaternion& rot) 
    : position(pos), rotation(rot) {}

Transform(const Vec3& pos, const Quaternion& rot, const Vec3& scl)
    : position(pos), rotation(rot), scale(scl) {}

void Translate(const Vec3& delta) {
    position += delta;
}

void Rotate(const Quaternion& delta) {
    rotation = rotation * delta;
}

void Scale(const Vec3& delta) {
    scale.x *= delta.x;
    scale.y *= delta.y;
    scale.z *= delta.z;
}

Vec3 Forward() const {
    return rotation * Vec3::Forward();
}

Vec3 Right() const {
    return rotation * Vec3::Right();
}

Vec3 Up() const {
    return rotation * Vec3::Up();
}
```

};

}
}
}
