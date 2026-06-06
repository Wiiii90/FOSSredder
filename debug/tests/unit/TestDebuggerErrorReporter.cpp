/**
 * @file debug/tests/unit/TestDebuggerErrorReporter.cpp
 * @brief Unit tests for the debugger-backed error reporter adapter.
 */

#include "gtest/gtest.h"

#include "core/errors/ErrorEvent.h"
#include "debug/DebugDefaults.h"
#include "debug/ErrorReporter.h"
#include "debug/IDebugger.h"

#include <memory>
#include <string>
#include <vector>

namespace {

class RecordingDebugger : public IDebugger {
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

TEST(DebuggerErrorReporterTests, ReportSkipsWritesWhenDebuggerIsDisabled)
{
    auto debugger = std::make_shared<RecordingDebugger>();
    debugger->enabledValue = false;
    debug::DebuggerErrorReporter reporter(debugger);

    core::errors::ErrorEvent event;
    event.message = "ignored";

    reporter.report(event);

    EXPECT_TRUE(debugger->lastPath.empty());
    EXPECT_TRUE(debugger->lastText.empty());
}

TEST(DebuggerErrorReporterTests, ReportFormatsSeverityCodeContextAndExceptionType)
{
    auto debugger = std::make_shared<RecordingDebugger>();
    debug::DebuggerErrorReporter reporter(debugger);

    core::errors::ErrorEvent event;
    event.severity = core::errors::ErrorSeverity::Warning;
    event.code = "DBG_TEST";
    event.origin = "debug::tests";
    event.message = "formatted message";
    event.context = {{"path", "demo.txt"}, {"stage", "unit"}};
    event.exceptionType = "DemoException";

    reporter.report(event);

    EXPECT_EQ(debugger->lastPath, std::string(debug::defaults::kDiagnosticsLogPath));
    EXPECT_NE(debugger->lastText.find("[warning]"), std::string::npos);
    EXPECT_NE(debugger->lastText.find("[DBG_TEST]"), std::string::npos);
    EXPECT_NE(debugger->lastText.find("debug::tests - formatted message"), std::string::npos);
    EXPECT_NE(debugger->lastText.find("path=demo.txt"), std::string::npos);
    EXPECT_NE(debugger->lastText.find("stage=unit"), std::string::npos);
    EXPECT_NE(debugger->lastText.find("(DemoException)"), std::string::npos);
}
