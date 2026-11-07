#include "imgui.h"
#include "Core/game.h"
#include "Features/prediction.h"
#include "Core/config.h"
#include <imgui.h>

void Prediction::DrawMenu() {
  auto& c = Config::Get();
  { float v = c.Get<float>("prediction.window_ms", 90.f); if (ImGui::SliderFloat("Lead ms", &v, 0.f, 200.f)) c.Set("prediction.window_ms", v); }
  bool m = c.Get<bool>("prediction.marker", true);
  if (ImGui::Checkbox("Draw marker", &m)) c.Set("prediction.marker", m);
}

void Prediction::Update(const Snapshot& s) {
  if (!enabled_) return;
  for (const auto& p : s.players) {
    auto& h = hist_[p.addr];
    h.push_back(p.head);
    while (h.size() > 8) h.pop_front();
  }
  if (hist_.size() > 96) hist_.clear();
}

Vec3 Prediction::Predict(uint64_t actor, Vec3 head, float dt) {
  auto it = hist_.find(actor);
  if (it == hist_.end() || it->second.size() < 4) return head;
  const auto& h = it->second;
  const Vec3 v = (h.back() - h[h.size() - 4]) * (1.f / 0.003f);
  return head + v * dt;
}

void Prediction::Render(const Snapshot& s) {
  if (!enabled_ || !s.valid) return;
  if (!Config::Get().Get<bool>("prediction.marker", true)) return;

  const float dt = Config::Get().Get<float>("prediction.window_ms", 90.f) / 1000.f;
  ImDrawList* dl = ImGui::GetBackgroundDrawList();

  for (const auto& p : s.players) {
    if (p.dead || p.team == s.localTeam) continue;
    const Vec3 pred = Predict(p.addr, p.head, dt);
    Vec2 sp;
    if (!c::WorldToScreen(pred, s.cam, s.pitch, s.yaw, s.fov, s.screen.x, s.screen.y, sp)) continue;
    dl->AddCircle(ImVec2(sp.x, sp.y), 6.f, IM_COL32(90, 200, 255, 235), 16, 1.8f);
  }
}



