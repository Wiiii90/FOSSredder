#pragma once

#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace persistence {
inline std::string generateUuid() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  uint64_t a = dis(gen);
  uint64_t b = dis(gen);
  std::ostringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(8)
     << static_cast<uint32_t>(a >> 32) << std::setw(4)
     << static_cast<uint16_t>((a >> 16) & 0xFFFF) << std::setw(4)
     << static_cast<uint16_t>(a & 0xFFFF) << std::setw(4)
     << static_cast<uint16_t>(b >> 48) << std::setw(12)
     << (b & 0xFFFFFFFFFFFFULL);
  std::string s = ss.str();
  // Format into 8-4-4-4-12
  if (s.size() >= 32) {
    return s.substr(0, 8) + "-" + s.substr(8, 4) + "-" + s.substr(12, 4) + "-" +
           s.substr(16, 4) + "-" + s.substr(20, 12);
  }
  return s;
}
} // namespace persistence
