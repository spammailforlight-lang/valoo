#include "Core/common.h"
#pragma once
#include "Core/common.h"
#include "Core/offsets.h"
#include <windows.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

struct Player {
  uint64_t addr = 0;
  Vec3 pos{};
  Vec3 head{};
  int team = 0;
  int health = 100;
  int agent = 0;
  int weapon = 0;
  bool spotted = false;
  bool dead = false;
};

struct Snapshot {
  std::vector<Player> players;
  Vec3 cam{};
  float pitch = 0, yaw = 0, fov = 90;
  Vec2 screen{1920, 1080};
  int localTeam = 0;
  bool valid = false;
  uint64_t tick = 0;
};

class Game {
public:
  static Game& Get();
  bool Attach(DWORD pid, const std::string& backend, const std::string& offsetPath);
  void Detach();
  bool Attached();
  Snapshot Read();
  Offsets& Off() { return off_; }

private:
  void Loop();
  Offsets off_;
  uint64_t base_ = 0;
  uint32_t size_ = 0;
  std::atomic<bool> run_{ false };
  std::atomic<bool> attached_{ false };
  std::thread th_;
  std::mutex mx_;
  Snapshot buf_[3];
  int cur_ = 0;
  bool buf_valid_ = false;
};

