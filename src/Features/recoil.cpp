#include "imgui.h"
#include "Core/game.h"
#include "Features/recoil.h"
#include "Core/config.h"
#include "Core/log.h"
#include <windows.h>
#include <fstream>
#include <nlohmann/json.hpp>

bool Recoil::Load(const std::string& path) {
  std::ifstream in(path);
  if (!in) return false;
  try {
    auto j = nlohmann::json::parse(in);
    for (auto& [k, v] : j.items()) {
      if (k.rfind("_", 0) == 0) continue;
      Curve cv;
      for (float f : v["dx"]) cv.dx.push_back(f);
      for (float f : v["dy"]) cv.dy.push_back(f);
      curves_[std::stoi(k)] = cv;
    }
    return true;
  } catch (const std::exception& e) {
    Log::Error(std::string("recoil parse: ") + e.what());
    return false;
  }
}

void Recoil::DrawMenu() {
  auto& c = Config::Get();
  { float v = c.Get<float>("recoil.multiplier", 1.f); if (ImGui::SliderFloat("Multiplier", &v, 0.f, 2.f)) c.Set("recoil.multiplier", v); }
  ImGui::TextDisabled("curves: %zu", curves_.size());
}

void Recoil::Update(const Snapshot& s) {
  if (!enabled_) return;
  if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) { shot_ = 0; return; }

  int weapon = 0;
  for (const auto& p : s.players)
    if (p.team == s.localTeam) { weapon = p.weapon; break; }
  if (!weapon) return;

  Fire(weapon);
  ++shot_;
}

void Recoil::Fire(int weapon) {
  auto it = curves_.find(weapon);
  if (it == curves_.end()) return;
  auto& cv = it->second;
  if (shot_ >= (int)cv.dy.size()) return;

  const float m = Config::Get().Get<float>("recoil.multiplier", 1.f);
  INPUT in{};
  in.type = INPUT_MOUSE;
  in.mi.dx = (LONG)lround(cv.dx[shot_] * m);
  in.mi.dy = (LONG)lround(cv.dy[shot_] * m);
  in.mi.dwFlags = MOUSEEVENTF_MOVE;
  SendInput(1, &in, sizeof(in));
}



