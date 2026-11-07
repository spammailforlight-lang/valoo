#include "Features/skins.h"
#include "Core/memory.h"
#include "Core/log.h"
#include <imgui.h>
#include <fstream>

bool Skins::Load(const std::string& path) {
  std::ifstream in(path);
  if (!in) return false;
  try { in >> db_; return true; }
  catch (const std::exception& e) { Log::Error(std::string("skins parse: ") + e.what()); return false; }
}

void Skins::DrawMenu() {
  if (db_.empty()) { ImGui::TextDisabled("skins.json not loaded"); return; }
  ImGui::TextWrapped("Client-side visual only. Invalid UUIDs fall back silently.");
  for (auto& [weapon, skins] : db_.items()) {
    if (weapon.rfind("_", 0) == 0) continue;
    if (!ImGui::TreeNode(weapon.c_str())) continue;
    for (auto& s : skins) {
      const std::string name = s.value("name", "?");
      if (ImGui::Selectable(name.c_str(), selected_ == s.value("uuid", ""))) selected_ = s.value("uuid", "");
    }
    ImGui::TreePop();
  }
}

void Skins::Apply(int weapon) {
  auto* p = Memory::Get().P();
  if (!p || !enabled_ || selected_.empty()) return;
  Log::Info("skin apply weapon=" + std::to_string(weapon) + " uuid=" + selected_);
}

