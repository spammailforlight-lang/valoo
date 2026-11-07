#include <windows.h>
#include <tlhelp32.h>
#include <memory>
#include <vector>
#include <string>
#include "Core/config.h"
#include "Core/log.h"
#include "Core/game.h"
#include "Core/hotkeys.h"
#include "Overlay/overlay.h"
#include "Features/radar.h"
#include "Features/prediction.h"
#include "Features/recoil.h"
#include "Features/movement.h"
#include "Features/utility.h"
#include "Features/skins.h"

static const wchar_t* GAME = L"VALORANT-Win64-Shipping.exe";

static DWORD FindGame() {
  DWORD pid = 0;
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return 0;
  PROCESSENTRY32W pe{ sizeof(pe) };
  if (Process32FirstW(snap, &pe)) do {
    if (!_wcsicmp(pe.szExeFile, GAME)) { pid = pe.th32ProcessID; break; }
  } while (Process32NextW(snap, &pe));
  CloseHandle(snap);
  return pid;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
  wchar_t dir[MAX_PATH]{};
  GetModuleFileNameW(nullptr, dir, MAX_PATH);
  std::wstring root(dir);
  root = root.substr(0, root.find_last_of(L'\\'));
  Log::Init(root + L"\\cheat.log");

  HANDLE once = CreateMutexW(nullptr, TRUE, L"CheatSingleInstance");
  if (!once) {
    Log::Warn("mutex create failed: " + std::to_string(GetLastError()));
  } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
    Log::Warn("already running");
    CloseHandle(once);
    Log::Shutdown();
    return 0;
  }
  Log::Info("start");

  Config::Get().Load();
  Hotkeys::Get().Bind("menu", Config::Get().Get<int>("menu_key", 0x2D));

  std::vector<std::unique_ptr<Feature>> features;
  auto radar = std::make_unique<Radar>();
  auto pred = std::make_unique<Prediction>();
  auto recoil = std::make_unique<Recoil>();
  auto move = std::make_unique<Movement>();
  auto util = std::make_unique<Utility>();
  auto skins = std::make_unique<Skins>();

  recoil->Load("data/recoil.json");
  skins->Load("data/skins.json");

  radar->SetEnabled(Config::Get().Get<bool>("radar.enabled", true));
  pred->SetEnabled(Config::Get().Get<bool>("prediction.enabled", true));
  recoil->SetEnabled(Config::Get().Get<bool>("recoil.enabled", false));
  move->SetEnabled(Config::Get().Get<bool>("movement.enabled", false));
  util->SetEnabled(Config::Get().Get<bool>("utility.enabled", true));
  skins->SetEnabled(Config::Get().Get<bool>("skins.enabled", false));

  features.push_back(std::move(radar));
  features.push_back(std::move(pred));
  features.push_back(std::move(recoil));
  features.push_back(std::move(move));
  features.push_back(std::move(util));
  features.push_back(std::move(skins));

  Overlay ov;
  try {
    if (!ov.Create(inst)) { Log::Error("overlay create failed"); return 1; }
    Log::Info("overlay up");
  } catch (const std::exception& e) {
    Log::Error("overlay exception: " + std::string(e.what()));
    return 2;
  }

  bool attached = false;
  MSG msg{};
  while (msg.message != WM_QUIT) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    const DWORD pid = FindGame();
    if (pid && !attached) {
      // Use config's backend setting, default to "driver" for Valorant
      auto b = Config::Get().Get<std::string>("backend", "driver");
      Log::Info("attaching to PID " + std::to_string(pid) + ", backend: " + b);
      if (Game::Get().Attach(pid, b, "data/offsets.json")) {
        attached = true;
        Log::Info("attached successfully");
      } else {
        Log::Error("attach failed");
      }
    }
    if (!pid && attached) { Game::Get().Detach(); attached = false; }

    if (Hotkeys::Get().Pressed("menu")) ov.ToggleMenu();

    Snapshot s;
    if (attached) {
      try {
        s = Game::Get().Read();
        for (auto& f : features) f->Update(s);
      } catch (const std::exception& e) {
        Log::Error("feature exception: " + std::string(e.what()));
      }
    }

    ov.Frame(features, s);
    Sleep(1);
  }

  // Save config with actual backend that worked
  Config::Get().Save();
  
  if (attached) {
    Log::Info("detaching from game...");
    Game::Get().Detach();
    Log::Info("game detached");
  }
  
  Log::Info("shutdown complete");
  ov.Destroy();
  if (once) CloseHandle(once);
  Log::Shutdown();
  return 0;
}

