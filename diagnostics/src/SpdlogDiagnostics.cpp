/**
 * @file diagnostics/src/SpdlogDiagnostics.cpp
 * @brief Implements a diagnostics backend that mirrors events to spdlog sinks.
 */

#include "diagnostics/pch.h"
#include "diagnostics/DiagnosticsDefaults.h"
#include "diagnostics/SpdlogDiagnostics.h"

#include <filesystem>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

namespace {

enum class ConsoleLevel {
    Info,
    Warning,
    Error,
    Critical
};

std::string_view trimLineEnd(std::string_view text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.remove_suffix(1);
    }
    return text;
}

ConsoleLevel consoleLevel(std::string_view text) {
    if (text.rfind("[critical]", 0) == 0) {
        return ConsoleLevel::Critical;
    }
    if (text.rfind("[error]", 0) == 0) {
        return ConsoleLevel::Error;
    }
    if (text.rfind("[warning]", 0) == 0) {
        return ConsoleLevel::Warning;
    }
    return ConsoleLevel::Info;
}

std::string_view stripSeverityPrefix(std::string_view text) {
    const auto line = trimLineEnd(text);
    const auto closing = line.find("] ");
    if (closing == std::string_view::npos || line.empty() || line.front() != '[') {
        return line;
    }
    return line.substr(closing + 2);
}

} // namespace

namespace diagnostics {

SpdlogDiagnostics::SpdlogDiagnostics(const std::string& loggerName, std::shared_ptr<core::ports::diagnostics::IDiagnostics> backend)
    : m_backend_(backend), m_enabled_(true)
{
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        const std::filesystem::path logPath(diagnostics::defaults::kSpdlogFilePath);
        if (logPath.has_parent_path()) std::filesystem::create_directories(logPath.parent_path());
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath.string(),
            diagnostics::defaults::kSpdlogFileSizeBytes,
            diagnostics::defaults::kSpdlogFileCount);
        spdlog::sinks_init_list sinks{console_sink, file_sink};
        m_logger_ = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
        spdlog::register_logger(m_logger_);
        m_logger_->set_level(spdlog::level::debug);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
    } catch (...) {
        m_enabled_ = false;
    }
}

SpdlogDiagnostics::~SpdlogDiagnostics() {
    try {
        if (m_logger_) spdlog::drop(m_logger_->name());
    } catch (...) {
        m_enabled_ = false;
    }
}

bool SpdlogDiagnostics::enabled() const {
    return m_enabled_;
}

void SpdlogDiagnostics::writeText(const std::string& relPath, const std::string& text) {
    if (!m_enabled_) return;

    try {
        if (relPath.rfind(std::string(diagnostics::defaults::kPopplerMetadataPrefix), 0) == 0) {
            if (m_backend_) {
                try { m_backend_->writeText(relPath, text); } catch (...) { if (m_logger_) m_logger_->warn("backend writeText failed for {}", relPath); }
            }
            if (m_logger_) m_logger_->debug("[{}] metadata written (content omitted)", relPath);
            return;
        }
    } catch (...) {
        if (m_logger_) m_logger_->warn("writeText pre-check failed for {}", relPath);
    }

    if (m_logger_) {
        const std::string_view line = trimLineEnd(text);
        const std::string_view message = stripSeverityPrefix(line);
        switch (consoleLevel(line)) {
        case ConsoleLevel::Critical:
            m_logger_->critical("{}", message);
            break;
        case ConsoleLevel::Error:
            m_logger_->error("{}", message);
            break;
        case ConsoleLevel::Warning:
            m_logger_->warn("{}", message);
            break;
        case ConsoleLevel::Info:
            m_logger_->info("{}", message);
            break;
        }
    }
    if (m_backend_) {
        try { m_backend_->writeText(relPath, text); } catch (...) { if (m_logger_) m_logger_->warn("backend writeText failed for {}", relPath); }
    }
}

void SpdlogDiagnostics::writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) {
    if (!m_enabled_) return;
    if (m_backend_) {
        try {
            m_backend_->writeBytes(relPath, data);
            if (m_logger_) m_logger_->debug("Wrote {} bytes to {} via backend", data.size(), relPath);
        } catch (...) {
            if (m_logger_) m_logger_->error("Failed to write bytes to {} via backend", relPath);
        }
    } else {
        if (m_logger_) m_logger_->debug("Received {} bytes for {} but no backend configured", data.size(), relPath);
    }
}

void SpdlogDiagnostics::flush() {
    if (m_backend_) {
        try { m_backend_->flush(); } catch (...) { if (m_logger_) m_logger_->warn("backend flush failed"); }
    }
    try { spdlog::drop_all(); } catch (...) { if (m_logger_) m_logger_->warn("spdlog drop_all failed"); }
}

} // namespace diagnostics
