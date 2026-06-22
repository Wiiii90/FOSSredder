#include "diagnostics/ErrorReporter.h"

#include "core/errors/ErrorReporting.h"
#include "diagnostics/DiagnosticsDefaults.h"
#include "diagnostics/FileDiagnostics.h"
#include "diagnostics/SpdlogDiagnostics.h"

#include <sstream>
#include <string>
#include <utility>

namespace diagnostics {

DiagnosticsErrorReporter::DiagnosticsErrorReporter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> diagnostics)
    : diagnostics_(std::move(diagnostics))
{
}

void DiagnosticsErrorReporter::report(const core::errors::ErrorEvent& event)
{
    if (!diagnostics_ || !diagnostics_->enabled()) return;

    std::ostringstream out;
    out << "[" << core::errors::severityToString(event.severity) << "] ";
    out << "[" << (event.code.empty() ? core::errors::codes::GenericError : event.code) << "] ";
    if (!event.origin.empty()) out << event.origin << " - ";
    out << event.message;
    if (!event.context.empty()) {
        out << " {";
        bool first = true;
        for (const auto& kv : event.context) {
            if (!first) out << ", ";
            first = false;
            out << kv.first << "=" << kv.second;
        }
        out << "}";
    }
    if (!event.exceptionType.empty()) out << " (" << event.exceptionType << ")";
    out << "\n";
    diagnostics_->writeText(std::string(diagnostics::defaults::kDiagnosticsLogPath), out.str());
}

std::shared_ptr<core::ports::diagnostics::IErrorReporter> createDefaultErrorReporter()
{
    auto fileBackend = std::make_shared<FileDiagnostics>(
        "", std::string(diagnostics::defaults::kDiagnosticsProcessName));
    return std::make_shared<DiagnosticsErrorReporter>(
        std::make_shared<SpdlogDiagnostics>(
            std::string(diagnostics::defaults::kDiagnosticsProcessName), std::move(fileBackend)));
}

}
