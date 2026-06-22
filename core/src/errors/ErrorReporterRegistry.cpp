#include "core/errors/ErrorReporterRegistry.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporting.h"

#include <mutex>

namespace core::errors {

namespace {

std::shared_ptr<core::ports::diagnostics::IErrorReporter> g_reporter;
std::mutex g_mutex;

}

void setGlobalErrorReporter(
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> reporter)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_reporter = std::move(reporter);
}

std::shared_ptr<core::ports::diagnostics::IErrorReporter> globalErrorReporter()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_reporter;
}

void report(const ErrorEvent& event)
{
    auto reporter = globalErrorReporter();
    if (reporter) reporter->report(event);
}

void report(ErrorSeverity severity, const char* code, const char* origin, std::string message, ErrorContext context)
{
    ErrorEvent event;
    event.severity = severity;
    event.code = code ? code : codes::GenericError;
    event.origin = origin ? origin : std::string{};
    event.message = std::move(message);
    event.context = std::move(context);
    report(event);
}

void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception)
{
    reportException(severity, codes::ExceptionError, origin, exception, {});
}

void reportException(ErrorSeverity severity, const char* code, const char* origin, std::exception_ptr exception, ErrorContext context)
{
    auto reporter = globalErrorReporter();
    if (!reporter) return;

    reporter->report(makeExceptionEvent(severity, code, origin, exception, std::move(context)));
}

}
