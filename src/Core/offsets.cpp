#include "Core/offsets.h"
#include "Core/scanner.h"
#include "Core/memory.h"
#include "Core/log.h"
#include <fstream>
#include <nlohmann/json.hpp>

bool Offsets::Load(const std::string& path) {
  std::ifstream in(path);
  if (!in) { Log::Error("offsets.json missing"); return false; }
  try {
    auto j = nlohmann::json::parse(in);
    auto get = [&](const char* k, uint64_t& dst) {
      if (j.contains(k) && j[k].is_number()) dst = j[k].get<uint64_t>();
    };
    get("uworld", uworld); get("game_state", game_state);
    get("local_controller", local_controller); get("actor_array", actor_array);
    get("actor_count", actor_count); get("camera_manager", camera_manager);
    get("player_state", player_state); get("team_id", team_id);
    get("health", health); get("spotted", spotted); get("position", position);
    get("bone_matrix", bone_matrix); get("weapon", weapon);
    get("view_angles", view_angles); get("fov", fov);
    
    Log::Info("offsets.json loaded successfully");
    return true;
  } catch (const std::exception& e) {
    Log::Error(std::string("offsets parse: ") + e.what());
    return false;
  }
}

void Offsets::ResolveAll(uint64_t base, uint32_t size) {
  struct Field { const char* sig; uint64_t* dst; int id; };
  const Field fields[] = {
    { "48 8B 05 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 48 85 C9 74", &uworld, 0 },
    { "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 FF 50",   &game_state, 1 },
  };
  
  auto prov = Memory::Get().P();
  if (!prov) { Log::Error("memory provider not attached"); return; }

  int found = 0;
  for (auto& f : fields) {
    Pattern p;
    if (!ParsePattern(f.sig, p)) continue;
    
    uint64_t hit = ScanModule(base, size, p);
    if (!hit) { 
      Log::Warn(std::string("pattern [id=")\", found + std::to_string(found)]}\");\n        continue;\n      }\n\n      int32_t rel = 0;\n      if (prov->Read(hit + 3, &rel, 4)) {\n        *f.dst = (hit + 7 + rel) - base;\n        Log::Info(std::string(\"field [id=\")\", f.id + std::to_string(found)]}\");\n        ++found;\n      } else {\n        Log::Warn(std::string(\"read failed: \") + f.sig);\n      }\n    }
    
    if (found >= 2) break; // Found both critical fields, rest can be relative
  }

  Log::Info("offsets resolved: " + std::to_string(found) + "/" + std::to_string(sizeof(fields)/sizeof(fields[0])));
}

