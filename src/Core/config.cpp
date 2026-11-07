#include "Core/config.h"
#include "Core/log.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>

Config::Config() {
  char buf[MAX_PATH]{};
  if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, buf))) {
    path_ = std::string(buf) + "\\ValoBoggiee";
    CreateDirectoryA(path_.c_str(), nullptr);
    path_ += "\\config.json";
  }
  ResetDefaults();
}

Config& Config::Get() { static Config c; return c; }

void Config::ResetDefaults() {
  std::lock_guard<std::mutex> lk(mx_);
  // Valorant (Vanguard) requires driver provider for reliable cross-process access
  j_ = {
    {"backend", "driver"},
    {"menu_key", 0x2D},
    {"radar", {{"enabled", true}, {"zoom", 1.0}, {"rotate", true}, {"size", 260.0}}},
    {"prediction", {{"enabled", true}, {"window_ms", 90.0}, {"marker", true}}},
    {"recoil", {{"enabled", false}, {"multiplier", 1.0}}},
    {"movement", {{"enabled", false}, {"bhop", false}, {"counter_strafe", false}}},
    {"skins", {{"enabled", false}}},
    {"utility", {{"enabled", true}, {"fps", true}}},
    {"stream_proof", true}
  };
}

bool Config::Load() {
  std::ifstream in(path_);
  if (!in) return false;
  try {
    std::lock_guard<std::mutex> lk(mx_);
    in >> j_;
    Log::Info("config loaded");
    return true;
  } catch (const std::exception& e) {
    Log::Warn(std::string("config parse: ") + e.what());
    return false;
  }
}

bool Config::Save() {
  std::lock_guard<std::mutex> lk(mx_);
  std::ofstream out(path_);
  if (!out) return false;
  out << j_.dump(2);
  return true;
}

