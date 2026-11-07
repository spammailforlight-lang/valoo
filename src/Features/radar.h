#pragma once
#include "Core/feature.h"

class Radar : public Feature {
public:
  const char* Name() const override { return "Radar"; }
  void DrawMenu() override;
  void Render(const Snapshot&) override;
};
