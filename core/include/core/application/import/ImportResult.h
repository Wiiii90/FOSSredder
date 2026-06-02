/**
 * @file core/include/core/application/import/ImportResult.h
 * @brief Declares the import execution result contract.
 */

#pragma once

#include "core/application/import/draft/TransactionDraft.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::application::importing {

struct ImportResult {
  std::shared_ptr<core::domain::Statement> data;
  std::vector<core::application::importing::draft::TransactionDraft>
      transactions;
  std::map<std::string, std::vector<std::uint8_t>> artifacts;
};

} // namespace core::application::importing
