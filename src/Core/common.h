#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace c {
constexpr float PI = 3.14159265358979f;

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };

inline Vec3 operator-(Vec3 a, Vec3 b) { return { a.x-b.x, a.y-b.y, a.z-b.z }; }
inline Vec3 operator+(Vec3 a, Vec3 b) { return { a.x+b.x, a.y+b.y, a.z+b.z }; }
inline Vec3 operator*(Vec3 a, float s) { return { a.x*s, a.y*s, a.z*s }; }
inline float Length(Vec3 v) { return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
inline Vec3 Normalize(Vec3 v) { float l = Length(v); return l > 0.0001f ? v * (1.f/l) : Vec3{0,0,0}; }
inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

inline Vec3 operator-(Vec3 v) { return { -v.x, -v.y, -v.z }; }
inline Vec3 operator*(float s, Vec3 v) { return { v.x*s, v.y*s, v.z*s }; }
inline Vec3 operator/(Vec3 v, float s) { return { v.x/s, v.y/s, v.z/s }; }
inline Vec3& operator+=(Vec3& a, Vec3 b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline float Dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 Cross(Vec3 a, Vec3 b) {
  return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
// World to screen. False when behind the camera.
bool WorldToScreen(Vec3 world, Vec3 cam, float pitch, float yaw, float fov,
                   float width, float height, Vec2& out);
}
using namespace c;
