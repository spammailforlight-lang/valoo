#include "imgui.h"
#include "Features/movement.h"
#include "Core/config.h"
#include <windows.h>

void Movement::DrawMenu() {
  auto& c = Config::Get();
  bool b = c.Get<bool>("movement.bhop", false);
  if (ImGui::Checkbox("Bunny hop", &b)) c.Set("movement.bhop", b);
  bool cs = c.Get<bool>("movement.counter_strafe", false);
  if (ImGui::Checkbox("Counter-strafe assist", &cs)) c.Set("movement.counter_strafe", cs);
}

namespace {
void Tap(int vk) {
  INPUT in{};
  in.type = INPUT_KEYBOARD;
  in.ki.wVk = (WORD)vk;
  SendInput(1, &in, sizeof(in));
  in.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &in, sizeof(in));
}
}

void Movement::Update(const Snapshot&) {
  if (!enabled_) return;

  if (Config::Get().Get<bool>("movement.bhop", false) && (GetAsyncKeyState(VK_SPACE) & 0x8000))
    Tap(VK_SPACE);

  if (Config::Get().Get<bool>("movement.counter_strafe", false)) {
    const bool a = (GetAsyncKeyState('A') & 0x8000) != 0;
    const bool d = (GetAsyncKeyState('D') & 0x8000) != 0;
    if (aWas_ && !a) Tap('D');
    if (dWas_ && !d) Tap('A');
    aWas_ = a; dWas_ = d;
  }
}


