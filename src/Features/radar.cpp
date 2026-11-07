#include "imgui.h"
#include "Core/game.h"
#include "Features/radar.h"
#include "Core/config.h"
#include <imgui.h>
#include <cmath>

void Radar::DrawMenu() {
  auto& c = Config::Get();
  { float v = c.Get<float>("radar.zoom", 1.f); if (ImGui::SliderFloat("Zoom", &v, 0.2f, 4.f)) c.Set("radar.zoom", v); }
  { float v = c.Get<float>("radar.size", 260.f); if (ImGui::SliderFloat("Size", &v, 160.f, 480.f)) c.Set("radar.size", v); }
  bool rot = c.Get<bool>("radar.rotate", true);
  if (ImGui::Checkbox("Rotate with player", &rot)) c.Set("radar.rotate", rot);
}

void Radar::Render(const Snapshot& s) {
  if (!enabled_ || !s.valid) return;

  ImGuiIO& io = ImGui::GetIO();
  ImDrawList* dl = ImGui::GetBackgroundDrawList();

  const float size = Config::Get().Get<float>("radar.size", 260.f);
  const float zoom = Config::Get().Get<float>("radar.zoom", 1.f);
  const float scale = (size * 0.5f) / (2600.f / zoom);
  const bool rotate = Config::Get().Get<bool>("radar.rotate", true);

  const ImVec2 ctr(io.DisplaySize.x - size * 0.5f - 24.f, io.DisplaySize.y - size * 0.5f - 24.f);

  dl->AddCircleFilled(ctr, size * 0.5f, IM_COL32(8, 10, 16, 190), 64);
  dl->AddCircle(ctr, size * 0.5f, IM_COL32(60, 70, 90, 220), 64, 1.5f);
  dl->AddCircleFilled(ctr, 3.f, IM_COL32(120, 220, 255, 255));

  for (const auto& p : s.players) {
    if (p.dead || p.team == s.localTeam) continue;

    float dx = (p.pos.x - s.cam.x) * scale;
    float dy = (p.pos.z - s.cam.z) * scale;
    if (rotate) {
      const float a = s.yaw * c::PI / 180.f;
      const float rx = dx * cosf(a) - dy * sinf(a);
      const float ry = dx * sinf(a) + dy * cosf(a);
      dx = rx; dy = ry;
    }

    const float d = sqrtf(dx * dx + dy * dy);
    if (d > size * 0.5f) {
      dx = dx / d * (size * 0.5f);
      dy = dy / d * (size * 0.5f);
    }

    const ImU32 col = p.spotted ? IM_COL32(255, 90, 90, 255) : IM_COL32(130, 60, 60, 210);
    const ImVec2 pt(ctr.x + dx, ctr.y + dy);
    dl->AddCircleFilled(pt, 4.f, col, 12);

    const float hp = c::Clamp(p.health / 100.f, 0.f, 1.f);
    dl->AddRectFilled(ImVec2(pt.x - 7, pt.y - 10), ImVec2(pt.x + 7, pt.y - 8), IM_COL32(40, 40, 40, 200));
    dl->AddRectFilled(ImVec2(pt.x - 7, pt.y - 10), ImVec2(pt.x - 7 + 14 * hp, pt.y - 8), IM_COL32(90, 220, 120, 230));
  }
}



