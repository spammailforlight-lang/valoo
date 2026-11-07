#pragma once
#include <string>
#include <unordered_map>

class Hotkeys {
public:
  static Hotkeys& Get();
  void Bind(const std::string& name, int vk);
  int  Vk(const std::string& name) const;
  bool Down(const std::string& name) const;
  bool Pressed(const std::string& name);
  std::string NameOf(int vk) const;
private:
  std::unordered_map<std::string, int> map_;
  std::unordered_map<std::string, bool> was_;
};
