/**
 * @file core/include/core/ports/usecases/import/ImportRequest.h
 * @brief Import request contract exposed through import ports.
 */

#pragma once

#include <string>

namespace core::ports::importing {

struct ImportRequest {
  std::string sourcePath;
};

} // namespace core::ports::importing
