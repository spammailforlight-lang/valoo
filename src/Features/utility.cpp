#include "imgui.h"
#include "Core/game.h"
#include "Features/utility.h"
#include "Core/config.h"
#include <imgui.h>

void Utility::DrawMenu() {
  auto& c = Config::Get();
  bool fps = c.Get<bool>("utility.fps", true);
  if (ImGui::Checkbox("Show counters", &fps)) c.Set("utility.fps", fps);
  ImGui::TextWrapped("Spike timer and ult tracking read from the same snapshot as the radar. "
                     "Extend offsets.json to populate them.");
}

void Utility::Render(const Snapshot& s) {
  if (!enabled_ || !Config::Get().Get<bool>("utility.fps", true)) return;
  ImDrawList* dl = ImGui::GetBackgroundDrawList();
  char buf[96];
  sprintf_s(buf, "actors %zu   tick %llu", s.players.size(), (unsigned long long)s.tick);
  dl->AddText(ImVec2(14, 14), IM_COL32(170, 200, 225, 220), buf);
}



