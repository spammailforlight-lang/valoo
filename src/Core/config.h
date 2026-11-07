#pragma once
#include <string>
#include <mutex>
#include <nlohmann/json.hpp>

class Config {
public:
  static Config& Get();
  bool Load();
  bool Save();
  void ResetDefaults();

  template <class T> T Get(const std::string& k, T def) {
    std::lock_guard<std::mutex> lk(mx_);
    try { return j_.value(k, def); } catch (...) { return def; }
  }
  template <class T> void Set(const std::string& k, T v) {
    std::lock_guard<std::mutex> lk(mx_);
    j_[k] = v;
  }
  std::string Path() const { return path_; }

private:
  Config();
  nlohmann::json j_;
  std::string path_;
  std::mutex mx_;
};
