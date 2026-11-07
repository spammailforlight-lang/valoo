#include "Core/scanner.h"
#include "Core/memory.h"
#include <algorithm>
#include <chrono>
#include <sstream>

bool ParsePattern(const std::string& text, Pattern& out) {
  std::istringstream ss(text);
  std::string tok;
  while (ss >> tok) {
    if (tok == "??" || tok == "?") { out.bytes.push_back(0); out.mask.push_back(0); }
    else {
      try { out.bytes.push_back((uint8_t)std::stoul(tok, nullptr, 16)); }
      catch (...) { return false; }
      out.mask.push_back(1);
    }
  }
  return out.bytes.size() >= 4;
}

uint64_t ScanModule(uint64_t base, uint32_t size, const Pattern& p, int timeout_ms) {
  auto* prov = Memory::Get().P();
  if (!prov || !base || !size || p.bytes.empty()) {
    return 0;
  }

  const size_t CHUNK = 0x100000;
  const size_t overlap = p.bytes.size();
  std::vector<uint8_t> buf(CHUNK + overlap);
  auto start = std::chrono::steady_clock::now();

  if (p.bytes.empty()) {
    return 0;
  }

  // Print first pattern for debug (first 3 patterns only)
  static int cnt = 0; if (cnt++ < 3) {
    std::ostringstream ss;
    for(size_t i=0;i<p.bytes.size();++i){ss << (p.mask[i] ? p.bytes[i] : "?");}
    Log::Info("scanning: " + ss.str());
  }

  for (size_t off = 0; off < size; off += CHUNK) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start).count() > timeout_ms) return 0;

    size_t want = (std::min)(CHUNK + overlap, (size_t)size - off);
    if (!prov->Read(base + off, buf.data(), want)) continue;

    for (size_t i = 0; i + p.bytes.size() <= want; ++i) {
      bool hit = true;
      for (size_t k = 0; k < p.bytes.size(); ++k)
        if (p.mask[k] && buf[i + k] != p.bytes[k]) { hit = false; break; }
      if (hit) return base + off + i;
    }
  }
  return 0;
}


