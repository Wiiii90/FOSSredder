#pragma once

#include "core/ports/diagnostics/IDiagnostics.h"
#include <memory>
#include <string>
#include <vector>

namespace spdlog { class logger; }

namespace diagnostics {

class SpdlogDiagnostics : public core::ports::diagnostics::IDiagnostics {
public:
    SpdlogDiagnostics(const std::string& loggerName, std::shared_ptr<core::ports::diagnostics::IDiagnostics> backend = nullptr);
    ~SpdlogDiagnostics() override;

    bool enabled() const override;
    void writeText(const std::string& relPath, const std::string& text) override;
    void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) override;
    void flush() override;

private:
    std::shared_ptr<spdlog::logger> m_logger_;
    std::shared_ptr<core::ports::diagnostics::IDiagnostics> m_backend_;
    bool m_enabled_;
};

} // namespace diagnostics
