#pragma once

#include <string>

namespace diagnostics {

// Start a new run for the given process name. Returns the run id string.
std::string startRun(const std::string& processName);

// Get current run id for this thread/process, empty if none.
std::string currentRun();

// End the current run (clear run id)
void endRun();

} // namespace diagnostics
