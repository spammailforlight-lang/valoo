#include "Core/hotkeys.h"
#include <windows.h>

Hotkeys& Hotkeys::Get() { static Hotkeys h; return h; }
void Hotkeys::Bind(const std::string& n, int vk) { map_[n] = vk; }
int  Hotkeys::Vk(const std::string& n) const { auto i = map_.find(n); return i == map_.end() ? 0 : i->second; }

bool Hotkeys::Down(const std::string& n) const {
  int vk = Vk(n);
  return vk && (GetAsyncKeyState(vk) & 0x8000);
}

bool Hotkeys::Pressed(const std::string& n) {
  int vk = Vk(n);
  if (!vk) return false;
  bool now = (GetAsyncKeyState(vk) & 0x8000) != 0;
  bool was = was_[n];
  was_[n] = now;
  return now && !was;
}

std::string Hotkeys::NameOf(int vk) const {
  char b[64]{};
  if (vk >= 0x70 && vk <= 0x87) { sprintf_s(b, "F%d", vk - 0x6F); return b; }
  if (GetKeyNameTextA(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16, b, 64) == 0) return "?";
  return b;
}

