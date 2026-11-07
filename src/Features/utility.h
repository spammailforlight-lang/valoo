#pragma once
#include "Core/feature.h"

class Utility : public Feature {
public:
  const char* Name() const override { return "Utility"; }
  void DrawMenu() override;
  void Render(const Snapshot&) override;
};
