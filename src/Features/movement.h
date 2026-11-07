#pragma once
#include "Core/feature.h"

class Movement : public Feature {
public:
  const char* Name() const override { return "Movement"; }
  void Update(const Snapshot&) override;
  void DrawMenu() override;
private:
  bool aWas_ = false, dWas_ = false;
};
