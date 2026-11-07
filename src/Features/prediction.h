#include "Core/common.h"
#pragma once
#include "Core/feature.h"
#include <deque>
#include <unordered_map>

class Prediction : public Feature {
public:
  const char* Name() const override { return "Head Prediction"; }
  void Update(const Snapshot&) override;
  void DrawMenu() override;
  void Render(const Snapshot&) override;
  Vec3 Predict(uint64_t actor, Vec3 head, float dt);
private:
  std::unordered_map<uint64_t, std::deque<Vec3>> hist_;
};

