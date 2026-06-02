/**
 * @file core/include/core/ports/import/ImportResult.h
 * @brief Import result contract exposed through import ports.
 */

#pragma once

#include "core/ports/import/ImportDraft.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace core::ports::importing {

struct ImportResult {
    core::ports::workspace::StatementSnapshot statement;
    bool hasStatement = false;
    std::vector<core::ports::importing::draft::TransactionDraft> transactions;
    std::map<std::string, std::vector<std::uint8_t>> artifacts;
};

} // namespace core::ports::importing
