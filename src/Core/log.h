#pragma once
#include <string>
namespace Log {
void Init(const std::wstring& path);
void Shutdown();
void Info(const std::string& m);
void Warn(const std::string& m);
void Error(const std::string& m);
}
