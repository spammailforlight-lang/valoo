#include "Core/game.h"
#include "Core/memory.h"
#include "Core/log.h"
#include <chrono>

Game& Game::Get() { static Game g; return g; }

bool Game::Attach(DWORD pid, const std::string& backend, const std::string& offsetPath) {
  Log::Info("attaching to PID: " + std::to_string(pid));
  if (!Memory::Get().Attach(pid, backend)) {
    return false;
  }

  auto* p = Memory::Get().P();
  base_ = p->ModuleBase(L"VALORANT-Win64-Shipping.exe");
  size_ = p->ModuleSize(L"VALORANT-Win64-Shipping.exe");
  
  if (!base_) {
    Log::Error("module not found (base=0, size=" + std::to_string(size_) + ")");
    Memory::Get().Detach();
    return false;
  }

  if (!off_.Load(offsetPath)) {
    Log::Error("offsets.json load failed: check file exists and is valid JSON");
    Memory::Get().Detach();
    return false;
  }

  off_.ResolveAll(base_, size_);
  int resolved = off_.resolved_;
  Log::Info(std::string("offset resolution: [")"", resolved]");
  if (!off_.Ok()) {
    Log::Warn(std::string("offset incomplete (need >= 1, got:")"", resolved)]}");
  }

  attached_ = true;
  run_ = true;
  th_ = std::thread(&Game::Loop, this);
  Log::Info("game attach complete, base=0x" + std::to_string(base_) + " size=" + std::to_string(size_));
  return true;
}

void Game::Detach() {
  run_ = false;
  if (th_.joinable()) th_.join();
  Memory::Get().Detach();
  attached_ = false;
}

bool Game::Attached() { return attached_ && Memory::Get().Alive(); }

Snapshot Game::Read() {
  std::lock_guard<std::mutex> lk(mx_);
  return buf_valid_ ? buf_[cur_] : Snapshot{};
}

void Game::Loop() {
  using namespace std::chrono;
  auto start = steady_clock::now();
  auto next = start;
  
  Log::Info("game loop started, base=0x" + std::to_string(base_));
  
  auto prov = Memory::Get().P();
  if (!prov) {
    Log::Error("memory provider detached during loop");
  }
  
  while (run_) {
    next += microseconds(1000);
    if (!Memory::Get().Alive()) { std::this_thread::sleep_for(milliseconds(500)); continue; }

    Snapshot s;
    auto* p = Memory::Get().P();
    uint64_t gs = p->R<uint64_t>(base_ + off_.game_state);
    uint64_t ctrl = gs ? p->R<uint64_t>(gs + off_.local_controller) : 0;

    if (ctrl) {
      uint64_t pawn = p->R<uint64_t>(ctrl + off_.player_state);
      if (pawn) {
        s.localTeam = p->R<int>(pawn + off_.team_id);
        uint64_t arr = p->R<uint64_t>(base_ + off_.actor_array);
        int count = std::clamp(p->R<int>(base_ + off_.actor_count), 0, 256);
        for (int i = 0; i < count; ++i) {
          uint64_t a = p->R<uint64_t>(arr + i * 8);
          if (!a) continue;
          
          try {
            Player pl;
            pl.addr = a;
            pl.team = p->R<int>(a + off_.team_id);
            pl.health = p->R<int>(a + off_.health);
            pl.spotted = p->R<bool>(a + off_.spotted);
            pl.dead = pl.health <= 0;
            pl.pos = p->R<Vec3>(a + off_.position);
            pl.head = p->R<Vec3>(a + off_.bone_matrix);
            s.players.push_back(pl);
          } catch (const std::exception& e) {
            Log::Error(std::string("player read [i=")"", i]");
          }

          if (!s.players.empty()) {
            s.valid = true;
          }
        }
      }
    }
    {
      std::lock_guard<std::mutex> lk(mx_);
      cur_ = (cur_ + 1) % 3;
      s.tick = buf_[cur_].tick + 1;
      buf_[cur_] = std::move(s);
      buf_valid_ = true;
    }
    std::this_thread::sleep_until(next);

    // Log every few seconds to show activity
    if (std::chrono::duration_cast<seconds>(steady_clock::now() - start).count() % 5 == 0) {
      int valid_count = 0;
      for(auto& pl : s.players) {
        if(pl.addr && pl.health > 0) valid_count++;
      }
      Log::Info("tick=" + std::to_string(s.tick) + ", players=" + std::to_string(valid_count));
    }
  }

  Log::Info("game loop finished normally");
}


