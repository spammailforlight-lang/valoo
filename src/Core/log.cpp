#include "Core/log.h"
#include <windows.h>
#include <mutex>
#include <cstdio>
#include <ctime>

namespace {
std::mutex g_mx;
FILE* g_f = nullptr;

void Write(const char* tag, const std::string& m) {
  std::lock_guard<std::mutex> lk(g_mx);
  if (!g_f) return;
  time_t t = time(nullptr);
  tm tmv{};
  localtime_s(&tmv, &t);
  char stamp[32];
  strftime(stamp, sizeof(stamp), "%H:%M:%S", &tmv);
  fprintf(g_f, "[%s] %-5s %s\n", stamp, tag, m.c_str());
  fflush(g_f);
  OutputDebugStringA(m.c_str());
  OutputDebugStringA("\n");
}
}

void Log::Init(const std::wstring& path) {
  std::lock_guard<std::mutex> lk(g_mx);
  _wfopen_s(&g_f, path.c_str(), L"a");
}
void Log::Shutdown() { std::lock_guard<std::mutex> lk(g_mx); if (g_f) { fclose(g_f); g_f = nullptr; } }
void Log::Info(const std::string& m) { Write("INFO", m); }
void Log::Warn(const std::string& m) { Write("WARN", m); }
void Log::Error(const std::string& m) { Write("ERROR", m); }

