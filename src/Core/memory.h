#include <string>
#pragma once
#include <windows.h>
#include <cstdint>
#include <memory>

class IMemoryProvider {
public:
  virtual ~IMemoryProvider() = default;
  virtual bool Attach(DWORD pid) = 0;
  virtual void Detach() = 0;
  virtual bool Alive() = 0;
  virtual bool Read(uint64_t addr, void* buf, size_t size) = 0;
  virtual uint64_t ModuleBase(const wchar_t* name) = 0;
  virtual uint32_t ModuleSize(const wchar_t* name) = 0;

  template <class T> T R(uint64_t a) { T v{}; Read(a, &v, sizeof(T)); return v; }
};

std::unique_ptr<IMemoryProvider> MakeHandleProvider();
std::unique_ptr<IMemoryProvider> MakeDriverProvider();

class Memory {
public:
  static Memory& Get();
  bool Attach(DWORD pid, const std::string& backend);
  void Detach();
  bool Alive();
  IMemoryProvider* P() { return prov_.get(); }
private:
  std::unique_ptr<IMemoryProvider> prov_;
};

