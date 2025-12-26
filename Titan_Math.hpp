#ifndef TITAN_MATH_HPP
#define TITAN_MATH_HPP
#include "Titan_Core.hpp"

namespace Titan::Math {
    static const float PI = 3.14159265359f;
    
    struct Vec2 { 
        float x=0, y=0; 
        Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
        Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
        Vec2 operator*(float s) const { return {x*s, y*s}; }
    };
    
    struct Vec3 { 
        float x=0, y=0, z=0; 
        Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
        Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
        Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    };
    
    struct Vec4 { float x=0, y=0, z=0, w=1; };

    struct Mat4 {
        float m[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
        static Mat4 Identity() { return Mat4(); }
        static Mat4 Ortho(float l, float r, float b, float t, float n, float f) {
            Mat4 res;
            res.m[0][0] = 2.0f/(r-l); res.m[1][1] = 2.0f/(t-b); res.m[2][2] = -2.0f/(f-n);
            res.m[3][0] = -(r+l)/(r-l); res.m[3][1] = -(t+b)/(t-b); res.m[3][2] = -(f+n)/(f-n);
            return res;
        }
        static Mat4 Translate(const Vec3& v) {
            Mat4 res; res.m[3][0]=v.x; res.m[3][1]=v.y; res.m[3][2]=v.z; return res;
        }
        static Mat4 Scale(const Vec3& v) {
            Mat4 res; res.m[0][0]=v.x; res.m[1][1]=v.y; res.m[2][2]=v.z; return res;
        }
        static Mat4 Mul(const Mat4& a, const Mat4& b) {
            Mat4 res = {0}; 
            for(int c=0;c<4;++c) for(int r=0;r<4;++r) for(int k=0;k<4;++k)
                res.m[c][r] += a.m[k][r] * b.m[c][k];
            return res;
        }
    };
}
#endif
