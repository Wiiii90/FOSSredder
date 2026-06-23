#pragma once

#include "core/ports/diagnostics/IDiagnostics.h"
#include <string>
#include <vector>
#include <mutex>

namespace diagnostics {

class FileDiagnostics : public core::ports::diagnostics::IDiagnostics {
public:
    // baseOrProcess: if second arg empty, this is treated as processName and base dir is ./diagnostics_output
    explicit FileDiagnostics(const std::string& baseOrProcess, const std::string& processName = "");
    ~FileDiagnostics() override;

    bool enabled() const override { return true; }
    void writeText(const std::string& relPath, const std::string& text) override;
    void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) override;
    void flush() override;

private:
    std::string baseDir_; // actual session directory used for writes
    std::string processName_;
    std::mutex mtx_;
};

} // namespace diagnostics
