#pragma once
#include "Core/feature.h"
#include <nlohmann/json.hpp>
#include <string>

class Skins : public Feature {
public:
  const char* Name() const override { return "Skin Changer"; }
  bool Load(const std::string& path);
  void DrawMenu() override;
  void Apply(int weapon);
private:
  nlohmann::json db_;
  std::string selected_;
};
