/**
 * @file diagnostics/src/FileDiagnostics.cpp
 * @brief Implements a diagnostics backend that writes artifacts to the filesystem.
 */

#include "diagnostics/pch.h"
#include "diagnostics/DiagnosticsDefaults.h"
#include "diagnostics/FileDiagnostics.h"
#include "diagnostics/RunManager.h"
#include <nlohmann/json.hpp>
#include <fstream>

static std::string make_session_dir(const std::string& base) {
    try {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char buf[128];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
        std::string session = std::string(buf);
        std::filesystem::path p = base;
        p /= session;
        std::filesystem::create_directories(p);
        return p.string();
    } catch (...) {
        return base;
    }
}

namespace diagnostics {

FileDiagnostics::FileDiagnostics(const std::string& baseOrProcess, const std::string& processName) : baseDir_(std::string()), processName_(std::string()) {
    try {
        std::filesystem::path base;
        std::string proc;
        if (processName.empty()) {
            base = std::filesystem::current_path() / diagnostics::defaults::kOutputDirectoryName;
            proc = baseOrProcess;
        } else {
            base = baseOrProcess.empty() ? (std::filesystem::current_path() / diagnostics::defaults::kOutputDirectoryName) : std::filesystem::path(baseOrProcess);
            proc = processName;
        }
        std::filesystem::create_directories(base);
        std::string sess = make_session_dir(base.string());
        baseDir_ = sess;
        processName_ = proc;
    } catch (...) {
        baseDir_ = (std::filesystem::current_path() / diagnostics::defaults::kOutputDirectoryName).string();
        processName_ = processName.empty() ? baseOrProcess : processName;
    }
}

FileDiagnostics::~FileDiagnostics() {
    flush();
}

static std::filesystem::path safe_join(const std::filesystem::path& base, const std::string& rel) {
    std::filesystem::path p(rel);
    std::filesystem::path clean;
    for (const auto& part : p) {
        if (part == "..") continue;
        if (part == "/" || part == "\\") continue;
        clean /= part;
    }
    return base / clean;
}

static std::string make_timestamp_filename(const std::string& name, const std::string& suffix = "") {
    try {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char buf[128];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
        std::ostringstream oss;
        oss << buf << "_" << std::setfill('0') << std::setw(3) << ms.count();
        if (!name.empty()) oss << "_" << name;
        return oss.str();
    } catch (...) {
        return name;
    }
}

void FileDiagnostics::writeText(const std::string& relPath, const std::string& text) {
    std::lock_guard<std::mutex> g(mtx_);
    try {
        // Do not persist short poppler log messages to files; they should appear on console only
        if (relPath.rfind(std::string(diagnostics::defaults::kPopplerLogPrefix), 0) == 0) return;

        auto runid = diagnostics::currentRun();
        if (runid.empty()) runid = diagnostics::startRun(processName_);
        auto rp = std::filesystem::path(relPath);
        auto rundir = safe_join(baseDir_, runid);
        std::filesystem::create_directories(rundir);

        std::string fname = make_timestamp_filename(rp.filename().string());
        auto outp = safe_join(rundir, fname);
        if (!outp.has_extension()) outp += ".txt";

        std::string outText = text;
        bool tryPretty = false;
        if (outp.has_extension() && outp.extension() == ".json") tryPretty = true;
        else {
            auto pos = text.find_first_not_of(" \t\r\n");
            if (pos != std::string::npos) {
                char c = text[pos];
                if (c == '{' || c == '[') tryPretty = true;
            }
        }

        if (tryPretty && nlohmann::json::accept(text)) {
            nlohmann::json j = nlohmann::json::parse(text);
            outText = j.dump(4);
        }

        std::ofstream ofs(outp, std::ios::binary);
        if (ofs) ofs << outText;
    } catch (...) {
        return;
    }
}

void FileDiagnostics::writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> g(mtx_);
    try {
        auto runid = diagnostics::currentRun();
        if (runid.empty()) runid = diagnostics::startRun(processName_);
        auto rp = std::filesystem::path(relPath);
        auto rundir = safe_join(baseDir_, runid);
        std::filesystem::create_directories(rundir);

        std::string fname = make_timestamp_filename(rp.filename().string());
        auto outp = safe_join(rundir, fname);
        if (!outp.has_extension()) outp += ".png";
        std::ofstream ofs(outp, std::ios::binary);
        if (ofs) ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    } catch (...) {
        return;
    }
}

void FileDiagnostics::flush() {
}

} // namespace diagnostics
