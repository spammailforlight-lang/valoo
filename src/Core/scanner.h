#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Pattern { std::vector<uint8_t> bytes; std::vector<uint8_t> mask; };

bool ParsePattern(const std::string& text, Pattern& out);
uint64_t ScanModule(uint64_t base, uint32_t size, const Pattern& p, int timeout_ms = 4000);
