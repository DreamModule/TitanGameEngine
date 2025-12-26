#ifndef TITAN_CORE_HPP
#define TITAN_CORE_HPP

#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <cstdio>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#else
#include <GLES3/gl3.h>
#endif

namespace Titan {

using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t; using u64 = uint64_t;
using i8 = int8_t; using i16 = int16_t; using i32 = int32_t; using i64 = int64_t;
using f32 = float; using f64 = double; using usize = size_t;

namespace Math {
    static constexpr f32 PI = 3.14159265359f;

    struct Vec2 { f32 x, y; };
    struct Vec3 { 
        f32 x, y, z; 
        static Vec3 Add(const Vec3& a, const Vec3& b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
        static Vec3 Mul(const Vec3& a, f32 s) { return {a.x*s, a.y*s, a.z*s}; }
    };
    struct Vec4 { f32 x, y, z, w; };

    struct Mat4 {
        f32 m[4][4];
        static Mat4 Identity() {
            Mat4 res = {0};
            res.m[0][0]=1; res.m[1][1]=1; res.m[2][2]=1; res.m[3][3]=1;
            return res;
        }
        static Mat4 Translate(const Vec3& t) {
            Mat4 res = Identity();
            res.m[3][0]=t.x; res.m[3][1]=t.y; res.m[3][2]=t.z;
            return res;
        }
        static Mat4 Scale(const Vec3& s) {
            Mat4 res = Identity();
            res.m[0][0]=s.x; res.m[1][1]=s.y; res.m[2][2]=s.z;
            return res;
        }
        static Mat4 Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
            Mat4 res = Identity();
            res.m[0][0]=2.0f/(r-l); res.m[1][1]=2.0f/(t-b); res.m[2][2]=-2.0f/(f-n);
            res.m[3][0]=-(r+l)/(r-l); res.m[3][1]=-(t+b)/(t-b); res.m[3][2]=-(f+n)/(f-n);
            return res;
        }
        static Mat4 Perspective(f32 fov, f32 ar, f32 n, f32 f) {
            Mat4 res = {0};
            f32 th = tanf(fov*0.5f);
            res.m[0][0]=1.0f/(ar*th); res.m[1][1]=1.0f/th;
            res.m[2][2]=-(f+n)/(f-n); res.m[2][3]=-1.0f;
            res.m[3][2]=-(2.0f*f*n)/(f-n);
            return res;
        }
        static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
             Vec3 f = {center.x-eye.x, center.y-eye.y, center.z-eye.z};
             f32 l = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
             f.x/=l; f.y/=l; f.z/=l;
             return Identity(); 
        }
        static Mat4 Mul(const Mat4& a, const Mat4& b) {
            Mat4 res = {0};
            for(int c=0;c<4;++c) for(int r=0;r<4;++r) for(int k=0;k<4;++k)
                res.m[c][r] += a.m[k][r] * b.m[c][k];
            return res;
        }
    };
}
namespace Data {
    struct Hash {
        static u32 FNV1a_32(const char* s) {
            u32 h=2166136261u;
            while(*s) { h^=(u8)*s++; h*=16777619u; }
            return h;
        }
    };
}
}
#endif
