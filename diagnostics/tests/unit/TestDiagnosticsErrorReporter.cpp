/**
 * @file diagnostics/tests/unit/TestDiagnosticsErrorReporter.cpp
 * @brief Unit tests for the diagnostics-backed error reporter adapter.
 */

#include "gtest/gtest.h"

#include "core/errors/ErrorEvent.h"
#include "diagnostics/DiagnosticsDefaults.h"
#include "diagnostics/ErrorReporter.h"
#include "core/ports/diagnostics/IDiagnostics.h"

#include <memory>
#include <string>
#include <vector>

namespace {

class RecordingDiagnostics : public core::ports::diagnostics::IDiagnostics {
public:
    bool enabledValue = true;
    std::string lastPath;
    std::string lastText;

    bool enabled() const override { return enabledValue; }

    void writeText(const std::string& relPath, const std::string& text) override
    {
        lastPath = relPath;
        lastText = text;
    }

    void writeBytes(const std::string&, const std::vector<uint8_t>&) override {}
};

} // namespace

TEST(DiagnosticsErrorReporterTests, ReportSkipsWritesWhenDiagnosticsIsDisabled)
{
    auto diagnosticsSink = std::make_shared<RecordingDiagnostics>();
    diagnosticsSink->enabledValue = false;
    diagnostics::DiagnosticsErrorReporter reporter(diagnosticsSink);

    core::errors::ErrorEvent event;
    event.message = "ignored";

    reporter.report(event);

    EXPECT_TRUE(diagnosticsSink->lastPath.empty());
    EXPECT_TRUE(diagnosticsSink->lastText.empty());
}

TEST(DiagnosticsErrorReporterTests, ReportFormatsSeverityCodeContextAndExceptionType)
{
    auto diagnosticsSink = std::make_shared<RecordingDiagnostics>();
    diagnostics::DiagnosticsErrorReporter reporter(diagnosticsSink);

    core::errors::ErrorEvent event;
    event.severity = core::errors::ErrorSeverity::Warning;
    event.code = "DBG_TEST";
    event.origin = "diagnostics::tests";
    event.message = "formatted message";
    event.context = {{"path", "demo.txt"}, {"stage", "unit"}};
    event.exceptionType = "DemoException";

    reporter.report(event);

    EXPECT_EQ(diagnosticsSink->lastPath, std::string(diagnostics::defaults::kDiagnosticsLogPath));
    EXPECT_NE(diagnosticsSink->lastText.find("[warning]"), std::string::npos);
    EXPECT_NE(diagnosticsSink->lastText.find("[DBG_TEST]"), std::string::npos);
    EXPECT_NE(diagnosticsSink->lastText.find("diagnostics::tests - formatted message"), std::string::npos);
    EXPECT_NE(diagnosticsSink->lastText.find("path=demo.txt"), std::string::npos);
    EXPECT_NE(diagnosticsSink->lastText.find("stage=unit"), std::string::npos);
    EXPECT_NE(diagnosticsSink->lastText.find("(DemoException)"), std::string::npos);
}
