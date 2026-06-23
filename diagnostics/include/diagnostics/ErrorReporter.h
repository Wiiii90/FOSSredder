/**
 * @file diagnostics/include/diagnostics/ErrorReporter.h
 * @brief Declares diagnostics-backed adapters for core error reporting ports.
 */

#pragma once

#include <memory>

#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/ports/diagnostics/IDiagnostics.h"

namespace diagnostics {

class DiagnosticsErrorReporter : public core::ports::diagnostics::IErrorReporter {
public:
    explicit DiagnosticsErrorReporter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> diagnostics);

    void report(const core::errors::ErrorEvent& event) override;

private:
    std::shared_ptr<core::ports::diagnostics::IDiagnostics> diagnostics_;
};

std::shared_ptr<core::ports::diagnostics::IErrorReporter> createDefaultErrorReporter();

}
