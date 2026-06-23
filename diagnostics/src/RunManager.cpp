#include "diagnostics/pch.h"
#include "diagnostics/RunManager.h"
#include <chrono>
#include <mutex>
#include <string>
#include <sstream>

namespace diagnostics {

namespace {
    std::mutex mtx;
    std::string current_run;
} // namespace

std::string startRun(const std::string& processName) {
    std::lock_guard<std::mutex> g(mtx);
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
    std::ostringstream oss; oss << buf << "_" << processName;
    current_run = oss.str();
    return current_run;
}

std::string currentRun() {
    std::lock_guard<std::mutex> g(mtx);
    return current_run;
}

void endRun() {
    std::lock_guard<std::mutex> g(mtx);
    current_run.clear();
}

} // namespace diagnostics
