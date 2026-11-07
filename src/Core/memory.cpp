#include "Core/memory.h"
#include "Core/log.h"
#include <psapi.h>
#include <string>

class HandleProvider : public IMemoryProvider {
  HANDLE h_ = nullptr;
public:
  ~HandleProvider() { Detach(); }
  bool Attach(DWORD pid) override {
    h_ = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    return h_ != nullptr;
  }
  void Detach() override { if (h_) { CloseHandle(h_); h_ = nullptr; } }
  bool Alive() override {
    if (!h_) return false;
    DWORD code = 0;
    return GetExitCodeProcess(h_, &code) && code == STILL_ACTIVE;
  }
  bool Read(uint64_t a, void* b, size_t s) override {
    return h_ && ReadProcessMemory(h_, (LPCVOID)a, b, s, nullptr);
  }
  uint64_t ModuleBase(const wchar_t* n) override {
    HMODULE mods[512]; DWORD need = 0;
    if (!EnumProcessModulesEx(h_, mods, sizeof(mods), &need, LIST_MODULES_ALL)) return 0;
    for (DWORD i = 0; i < need / sizeof(HMODULE); ++i) {
      wchar_t name[MAX_PATH]{};
      if (GetModuleBaseNameW(h_, mods[i], name, MAX_PATH) && _wcsicmp(name, n) == 0)
        return (uint64_t)mods[i];
    }
    return 0;
  }
  uint32_t ModuleSize(const wchar_t* n) override {
    uint64_t b = ModuleBase(n);
    if (!b) return 0;
    MODULEINFO mi{};
    if (!GetModuleInformation(h_, (HMODULE)b, &mi, sizeof(mi))) return 0;
    return mi.SizeOfImage;
  }
};

class DriverProvider : public IMemoryProvider {
  HANDLE h_ = INVALID_HANDLE_VALUE;
  DWORD pid_ = 0;
  uint64_t base_ = 0;
  uint32_t size_ = 0;

  struct Req { uint64_t addr; uint64_t buf; uint32_t size; uint32_t pad; };
  static constexpr DWORD IO_READ = 0x22E040;

public:
  ~DriverProvider() { Detach(); }
  bool Attach(DWORD pid) override {
    h_ = CreateFileW(L"\\\\.\\iqvw64e", GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h_ == INVALID_HANDLE_VALUE) { Log::Error("iqvw64e open failed"); return false; }
    pid_ = pid;
    auto hp = MakeHandleProvider();
    if (hp->Attach(pid)) {
      base_ = hp->ModuleBase(L"VALORANT-Win64-Shipping.exe");
      size_ = hp->ModuleSize(L"VALORANT-Win64-Shipping.exe");
    }
    return base_ != 0;
  }
  void Detach() override {
    if (h_ != INVALID_HANDLE_VALUE) { CloseHandle(h_); h_ = INVALID_HANDLE_VALUE; }
  }
  bool Alive() override {
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid_);
    if (!p) return false;
    DWORD c = 0; bool ok = GetExitCodeProcess(p, &c) && c == STILL_ACTIVE;
    CloseHandle(p);
    return ok;
  }
  bool Read(uint64_t a, void* b, size_t s) override {
    if (h_ == INVALID_HANDLE_VALUE) { return false; }
    
    Req r{ a, (uint64_t)b, (uint32_t)s, 0 };
    DWORD ret = 0;
    bool ok = DeviceIoControl(h_, IO_READ, &r, sizeof(r), &r, sizeof(r), &ret, nullptr);
    return ok;
  }
  uint64_t ModuleBase(const wchar_t*) override { return base_; }
  uint32_t ModuleSize(const wchar_t*) override { return size_; }
};

std::unique_ptr<IMemoryProvider> MakeHandleProvider() { return std::make_unique<HandleProvider>(); }
std::unique_ptr<IMemoryProvider> MakeDriverProvider() { return std::make_unique<DriverProvider>(); }

Memory& Memory::Get() { static Memory m; return m; }

bool Memory::Attach(DWORD pid, const std::string& backend) {
  // If explicit driver mode requested, use it
  if (backend == "driver") {
    prov_ = MakeDriverProvider();
    if (!prov_->Attach(pid)) { prov_.reset(); return false; }
    Log::Info("attached via driver");
    return true;
  }
  
  // Try HandleProvider first (works for most games)
  auto hp = MakeHandleProvider();
  if (hp->Attach(pid)) {
    prov_ = std::move(hp);
    Log::Info("attached via handle provider");
    return true;
  }
  
  // If HandleProvider fails, fall back to DriverProvider for elevated/Vanguard-protected processes
  Log::Warn("HandleProvider failed (likely non-elevated parent), trying DriverProvider fallback...");
  prov_ = MakeDriverProvider();
  if (!prov_->Attach(pid)) { prov_.reset(); return false; }
  Log::Info("attached via driver provider (fallback)");
  return true;
}
void Memory::Detach() { if (prov_) { prov_->Detach(); prov_.reset(); } }
bool Memory::Alive() { return prov_ && prov_->Alive(); }

