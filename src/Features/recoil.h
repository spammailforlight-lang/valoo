#include <string>
#pragma once
#include "Core/feature.h"
#include <vector>
#include <unordered_map>

struct Curve { std::vector<float> dx, dy; };

class Recoil : public Feature {
public:
  const char* Name() const override { return "Recoil Control"; }
  bool Load(const std::string& path);
  void Update(const Snapshot&) override;
  void DrawMenu() override;
private:
  void Fire(int weapon);
  std::unordered_map<int, Curve> curves_;
  int shot_ = 0;
};

