#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::diagnostics {

class IDiagnostics {
public:
  virtual ~IDiagnostics() = default;

  virtual bool enabled() const = 0;
  virtual void writeText(const std::string &relPath, const std::string &text) = 0;
  virtual void writeBytes(const std::string &relPath,
                          const std::vector<std::uint8_t> &data) = 0;
  virtual void flush() {}
};

} // namespace core::ports::diagnostics
