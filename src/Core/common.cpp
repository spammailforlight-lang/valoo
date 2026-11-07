#include "Core/common.h"

bool c::WorldToScreen(Vec3 w, Vec3 cam, float pitch, float yaw, float fov,
                      float width, float height, Vec2& out) {
  const float pr = pitch * PI / 180.f, yr = yaw * PI / 180.f;
  const float cp = cosf(pr), sp = sinf(pr), cy = cosf(yr), sy = sinf(yr);

  const Vec3 fwd{ cp * cy, cp * sy, sp };
  const Vec3 right{ -sy, cy, 0.f };
  const Vec3 up{ -sp * cy, -sp * sy, cp };

  const Vec3 d = w - cam;
  const float z = d.x * fwd.x + d.y * fwd.y + d.z * fwd.z;
  if (z <= 0.1f) return false;

  const float x = d.x * right.x + d.y * right.y + d.z * right.z;
  const float y = d.x * up.x + d.y * up.y + d.z * up.z;
  const float t = tanf(fov * 0.5f * PI / 180.f);

  out.x = width * 0.5f + (x / (z * t)) * (width * 0.5f);
  out.y = height * 0.5f - (y / (z * t)) * (width * 0.5f);
  return true;
}

