#pragma once
#include "Core/common.h"

struct Snapshot;

class Feature {
public:
  virtual ~Feature() = default;
  virtual const char* Name() const = 0;
  virtual void Update(const Snapshot&) {}
  virtual void DrawMenu() {}
  virtual void Render(const Snapshot&) {}
  bool Enabled() const { return enabled_; }
  void SetEnabled(bool v) { enabled_ = v; }
protected:
  bool enabled_ = false;
};
