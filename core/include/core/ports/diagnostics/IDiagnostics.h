/**
 * @file core/include/core/ports/diagnostics/IDiagnostics.h
 * @brief Declares the diagnostics artifact writing port.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::diagnostics {

/**
 * @brief Port for writing optional diagnostic artifacts.
 */
class IDiagnostics {
public:
  /**
   * @brief Destroy the diagnostics port.
   */
  virtual ~IDiagnostics() = default;

  /**
   * @brief Check whether diagnostic artifact writing is enabled.
   * @return True when diagnostics should be written.
   */
  virtual bool enabled() const = 0;

  /**
   * @brief Write a text diagnostic artifact.
   * @param relPath Relative artifact path.
   * @param text Text content to write.
   */
  virtual void writeText(const std::string &relPath, const std::string &text) = 0;

  /**
   * @brief Write a binary diagnostic artifact.
   * @param relPath Relative artifact path.
   * @param data Binary content to write.
   */
  virtual void writeBytes(const std::string &relPath,
                          const std::vector<std::uint8_t> &data) = 0;

  /**
   * @brief Flush pending diagnostic output.
   */
  virtual void flush() {}
};

} // namespace core::ports::diagnostics
