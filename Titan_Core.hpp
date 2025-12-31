/**
 * Titan Core Header
 * 
 * Basic types, math structures, and utilities
 * Follows Unreal Engine naming conventions
 */

#ifndef TITAN_CORE_HPP
#define TITAN_CORE_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <vector>
#include <type_traits>

namespace Titan {

// ============================================================================
// Type Aliases
// ============================================================================

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using int8   = int8_t;
using int16  = int16_t;
using int32  = int32_t;
using int64  = int64_t;

// Legacy aliases for compatibility
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using usize = size_t;

// ============================================================================
// Hashing Utilities
// ============================================================================

namespace Data {
namespace Hash {

/**
 * Compile-time FNV-1a hash
 */
constexpr uint32 FNV1a_32(const char* String)
{
    uint32 Hash = 0x811c9dc5;
    while (*String)
    {
        Hash = (Hash ^ static_cast<uint32>(*String)) * 0x01000193;
        ++String;
    }
    return Hash;
}

/**
 * 64-bit FNV-1a hash
 */
constexpr uint64 FNV1a_64(const char* String)
{
    uint64 Hash = 0xcbf29ce484222325ULL;
    while (*String)
    {
        Hash = (Hash ^ static_cast<uint64>(*String)) * 0x100000001b3ULL;
        ++String;
    }
    return Hash;
}

} // namespace Hash
} // namespace Data

// ============================================================================
// Snapshot Storage
// ============================================================================

struct FSnapshotStorage
{
    std::vector<uint8> Buffer;
    uint32 WritePos = 0;
    uint32 ReadPos = 0;

    void Reset()
    {
        WritePos = 0;
        ReadPos = 0;
        std::vector<uint8>().swap(Buffer);
    }

    template<typename T>
    void Write(const T& Data)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Snapshot: POD types only!");
        usize Size = sizeof(T);
        if (WritePos + Size > Buffer.size())
        {
            Buffer.resize(WritePos + Size + 1024);
        }
        std::memcpy(&Buffer[WritePos], &Data, Size);
        WritePos += static_cast<uint32>(Size);
    }

    template<typename T>
    bool Read(T& OutData)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Snapshot: POD types only!");
        if (ReadPos + sizeof(T) > WritePos)
        {
            return false;
        }
        std::memcpy(&OutData, &Buffer[ReadPos], sizeof(T));
        ReadPos += static_cast<uint32>(sizeof(T));
        return true;
    }

    const uint8* GetData() const { return Buffer.data(); }
    uint32 GetSize() const { return WritePos; }
};

// Legacy alias
using SnapshotStorage = FSnapshotStorage;

// ============================================================================
// Math Types
// ============================================================================

namespace Math {

static constexpr float PI = 3.14159265358979323846f;
static constexpr float TAU = 6.28318530717958647692f;
static constexpr float DEG_TO_RAD = PI / 180.0f;
static constexpr float RAD_TO_DEG = 180.0f / PI;

/**
 * 2D Vector
 */
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float InX, float InY) : x(InX), y(InY) {}

    static Vec2 Zero() { return {0.0f, 0.0f}; }
    static Vec2 One() { return {1.0f, 1.0f}; }
    static Vec2 Up() { return {0.0f, -1.0f}; }
    static Vec2 Down() { return {0.0f, 1.0f}; }
    static Vec2 Left() { return {-1.0f, 0.0f}; }
    static Vec2 Right() { return {1.0f, 0.0f}; }

    Vec2 operator+(const Vec2& Other) const { return {x + Other.x, y + Other.y}; }
    Vec2 operator-(const Vec2& Other) const { return {x - Other.x, y - Other.y}; }
    Vec2 operator*(float Scalar) const { return {x * Scalar, y * Scalar}; }
    Vec2 operator/(float Scalar) const { return {x / Scalar, y / Scalar}; }
    Vec2& operator+=(const Vec2& Other) { x += Other.x; y += Other.y; return *this; }
    Vec2& operator-=(const Vec2& Other) { x -= Other.x; y -= Other.y; return *this; }
    Vec2& operator*=(float Scalar) { x *= Scalar; y *= Scalar; return *this; }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }
    float Dot(const Vec2& Other) const { return x * Other.x + y * Other.y; }
    
    Vec2 Normalized() const
    {
        float Len = Length();
        if (Len > 0.0001f)
        {
            return {x / Len, y / Len};
        }
        return *this;
    }
};

/**
 * 3D Vector
 */
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float InX, float InY, float InZ) : x(InX), y(InY), z(InZ) {}

    static Vec3 Zero() { return {0.0f, 0.0f, 0.0f}; }
    static Vec3 One() { return {1.0f, 1.0f, 1.0f}; }
    static Vec3 Up() { return {0.0f, 1.0f, 0.0f}; }
    static Vec3 Down() { return {0.0f, -1.0f, 0.0f}; }
    static Vec3 Left() { return {-1.0f, 0.0f, 0.0f}; }
    static Vec3 Right() { return {1.0f, 0.0f, 0.0f}; }
    static Vec3 Forward() { return {0.0f, 0.0f, 1.0f}; }
    static Vec3 Back() { return {0.0f, 0.0f, -1.0f}; }

    Vec3 operator+(const Vec3& Other) const { return {x + Other.x, y + Other.y, z + Other.z}; }
    Vec3 operator-(const Vec3& Other) const { return {x - Other.x, y - Other.y, z - Other.z}; }
    Vec3 operator*(float Scalar) const { return {x * Scalar, y * Scalar, z * Scalar}; }
    Vec3 operator/(float Scalar) const { return {x / Scalar, y / Scalar, z / Scalar}; }
    Vec3& operator+=(const Vec3& Other) { x += Other.x; y += Other.y; z += Other.z; return *this; }
    Vec3& operator-=(const Vec3& Other) { x -= Other.x; y -= Other.y; z -= Other.z; return *this; }
    Vec3& operator*=(float Scalar) { x *= Scalar; y *= Scalar; z *= Scalar; return *this; }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }
    float Dot(const Vec3& Other) const { return x * Other.x + y * Other.y + z * Other.z; }
    
    Vec3 Cross(const Vec3& Other) const
    {
        return {
            y * Other.z - z * Other.y,
            z * Other.x - x * Other.z,
            x * Other.y - y * Other.x
        };
    }

    Vec3 Normalized() const
    {
        float Len = Length();
        if (Len > 0.0001f)
        {
            return {x / Len, y / Len, z / Len};
        }
        return *this;
    }
};

/**
 * 4D Vector / Color
 */
struct Vec4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    Vec4() = default;
    Vec4(float InX, float InY, float InZ, float InW) : x(InX), y(InY), z(InZ), w(InW) {}

    static Vec4 Zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    static Vec4 One() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static Vec4 White() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static Vec4 Black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static Vec4 Red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static Vec4 Green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static Vec4 Blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }

    Vec4 operator+(const Vec4& Other) const { return {x + Other.x, y + Other.y, z + Other.z, w + Other.w}; }
    Vec4 operator*(float Scalar) const { return {x * Scalar, y * Scalar, z * Scalar, w * Scalar}; }
};

/**
 * 4x4 Matrix (column-major)
 */
struct Mat4
{
    float m[4][4] = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};

    static Mat4 Identity()
    {
        return Mat4();
    }

    static Mat4 Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far)
    {
        Mat4 Result;
        Result.m[0][0] = 2.0f / (Right - Left);
        Result.m[1][1] = 2.0f / (Top - Bottom);
        Result.m[2][2] = -2.0f / (Far - Near);
        Result.m[3][0] = -(Right + Left) / (Right - Left);
        Result.m[3][1] = -(Top + Bottom) / (Top - Bottom);
        Result.m[3][2] = -(Far + Near) / (Far - Near);
        return Result;
    }

    static Mat4 Perspective(float FOV, float Aspect, float Near, float Far)
    {
        Mat4 Result = {};
        float TanHalfFOV = std::tan(FOV * 0.5f * DEG_TO_RAD);
        
        Result.m[0][0] = 1.0f / (Aspect * TanHalfFOV);
        Result.m[1][1] = 1.0f / TanHalfFOV;
        Result.m[2][2] = -(Far + Near) / (Far - Near);
        Result.m[2][3] = -1.0f;
        Result.m[3][2] = -(2.0f * Far * Near) / (Far - Near);
        Result.m[3][3] = 0.0f;
        
        return Result;
    }

    static Mat4 Translate(const Vec3& V)
    {
        Mat4 Result;
        Result.m[3][0] = V.x;
        Result.m[3][1] = V.y;
        Result.m[3][2] = V.z;
        return Result;
    }

    static Mat4 Scale(const Vec3& V)
    {
        Mat4 Result;
        Result.m[0][0] = V.x;
        Result.m[1][1] = V.y;
        Result.m[2][2] = V.z;
        return Result;
    }

    static Mat4 RotateZ(float Radians)
    {
        Mat4 Result;
        float C = std::cos(Radians);
        float S = std::sin(Radians);
        Result.m[0][0] = C;
        Result.m[0][1] = S;
        Result.m[1][0] = -S;
        Result.m[1][1] = C;
        return Result;
    }

    static Mat4 Mul(const Mat4& A, const Mat4& B)
    {
        Mat4 Result = {};
        for (int C = 0; C < 4; ++C)
        {
            for (int R = 0; R < 4; ++R)
            {
                for (int K = 0; K < 4; ++K)
                {
                    Result.m[C][R] += A.m[K][R] * B.m[C][K];
                }
            }
        }
        return Result;
    }

    Mat4 operator*(const Mat4& Other) const
    {
        return Mul(*this, Other);
    }
};

/**
 * Quaternion
 */
struct Quaternion
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    Quaternion() = default;
    Quaternion(float InX, float InY, float InZ, float InW) : x(InX), y(InY), z(InZ), w(InW) {}

    static Quaternion Identity() { return {0.0f, 0.0f, 0.0f, 1.0f}; }

    static Quaternion FromEuler(float Pitch, float Yaw, float Roll)
    {
        float CX = std::cos(Pitch * 0.5f);
        float SX = std::sin(Pitch * 0.5f);
        float CY = std::cos(Yaw * 0.5f);
        float SY = std::sin(Yaw * 0.5f);
        float CZ = std::cos(Roll * 0.5f);
        float SZ = std::sin(Roll * 0.5f);

        return {
            SX * CY * CZ - CX * SY * SZ,
            CX * SY * CZ + SX * CY * SZ,
            CX * CY * SZ - SX * SY * CZ,
            CX * CY * CZ + SX * SY * SZ
        };
    }

    Quaternion operator*(const Quaternion& Other) const
    {
        return {
            w * Other.x + x * Other.w + y * Other.z - z * Other.y,
            w * Other.y - x * Other.z + y * Other.w + z * Other.x,
            w * Other.z + x * Other.y - y * Other.x + z * Other.w,
            w * Other.w - x * Other.x - y * Other.y - z * Other.z
        };
    }

    Vec3 Rotate(const Vec3& V) const
    {
        Vec3 U(x, y, z);
        float S = w;
        return U * 2.0f * U.Dot(V) + V * (S * S - U.Dot(U)) + U.Cross(V) * 2.0f * S;
    }
};

} // namespace Math

} // namespace Titan

#endif // TITAN_CORE_HPP
