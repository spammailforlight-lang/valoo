#pragma once
#include <cstdint>
#include <string>

struct Offsets {
  uint64_t uworld = 0;
  uint64_t game_state = 0;
  uint64_t local_controller = 0;
  uint64_t actor_array = 0;
  uint64_t actor_count = 0;
  uint64_t camera_manager = 0;
  uint64_t player_state = 0;
  uint64_t team_id = 0;
  uint64_t health = 0;
  uint64_t spotted = 0;
  uint64_t position = 0;
  uint64_t bone_matrix = 0;
  uint64_t weapon = 0;
  uint64_t view_angles = 0;
  uint64_t fov = 0;

  bool Load(const std::string& path);
  void ResolveAll(uint64_t moduleBase, uint32_t moduleSize);
  bool Ok() const { return resolved_ >= 1; } // At least one critical field (uworld/game_state)
  int resolved_ = 0;
};
